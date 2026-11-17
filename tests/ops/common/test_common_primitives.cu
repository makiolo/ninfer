// R97-09 oracle: acceptance test for the src/ops/common primitives on the HIP/wave32 port.
// Calls the primitives by their CUDA / ninfer::ops::common names; expectations come from
// primitives_reference.h (mathematical definitions), never from the implementation under test.
// Not registered in CMake (the author of the port does that). Standalone executable, no GPU
// framework: `--no-pinned` skips the PINNED tier, `--filter TEXT` keeps tests whose name contains TEXT.
// Contract assumed for the port (see the report): names and semantics in reduce.cuh / memory.cuh unchanged,
// kWarpSize == 32 == hardware warpSize, __ballot_sync returns a 32-bit mask with zero upper bits.
#include "core/cuda_compat.h"
#include "ops/common/cuda/memory.cuh"
#include "ops/common/cuda/reduce.cuh"
#include "primitives_reference.h"

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>
#include <type_traits>
#include <vector>

using namespace ninfer::ops::common;
using namespace ninfer::test::common_ref;

static_assert(kWarpSize == 32 && kFullWarpMask == 0xffffffffu);

namespace {

#define CK(call) do { cudaError_t e_ = (call); if (e_ != cudaSuccess) { std::fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, cudaGetErrorString(e_)); std::exit(2); } } while (0)

int g_fail = 0, g_checks = 0;
bool g_pinned = true;
std::string g_filter, g_current;
bool skip(const char* name) { g_current = name; return !g_filter.empty() && g_current.find(g_filter) == std::string::npos; }
void fail(const char* tier, const std::string& what, const std::string& why) {
    ++g_fail;
    if (g_fail <= 60) std::printf("FAIL [%s] %s :: %s :: %s\n", tier, g_current.c_str(), what.c_str(), why.c_str());
}
void check(bool ok, const char* tier, const std::string& what, const std::string& why = "") { ++g_checks; if (!ok) fail(tier, what, why); }
std::string hex(unsigned long long v) { char b[24]; std::snprintf(b, sizeof b, "0x%llx", v); return b; }

template <class T> struct Dev {
    T* p = nullptr; std::size_t n;
    explicit Dev(std::size_t count) : n(count) { CK(cudaMalloc(&p, n * sizeof(T))); CK(cudaMemset(p, 0, n * sizeof(T))); }
    Dev(const std::vector<T>& h) : Dev(h.size()) { CK(cudaMemcpy(p, h.data(), n * sizeof(T), cudaMemcpyHostToDevice)); }
    ~Dev() { cudaFree(p); }
    std::vector<T> get() const { std::vector<T> h(n); CK(cudaMemcpy(h.data(), p, n * sizeof(T), cudaMemcpyDeviceToHost)); return h; }
};
void sync() { CK(cudaGetLastError()); CK(cudaDeviceSynchronize()); }

// Instantiate only the listed sizes: a runtime switch also instantiates invalid
// warp widths (>32) and invalid block sizes (<32), even when never selected.
template <int... Sizes, class F> void with_pow2(F&& f) {
    (f(std::integral_constant<int, Sizes>{}), ...);
}
bool same_bits_or_nan(float a, float b) { return (std::isnan(a) && std::isnan(b)) || bits_of(a) == bits_of(b); }

// ============================================================================ warp_sum / warp_reduce_sum / warp_max
template <int W> __global__ void k_warp(const float* s, const float* m, float* xr, float* dn, float* mx) {
    const int l = threadIdx.x;
    xr[l] = warp_sum<W>(s[l]);
    dn[l] = warp_reduce_sum<W>(s[l]);
    mx[l] = warp_max<W>(m[l]);
}
// Only the lanes named by `mask` run the reduction (partial-mask shuffles, Width 8).
template <unsigned Mask> __global__ void k_warp_partial(const float* s, float* xr, float* dn) {
    const int l = threadIdx.x;
    if ((Mask >> l) & 1u) { xr[l] = warp_sum<8>(s[l], Mask); dn[l] = warp_reduce_sum<8>(s[l], Mask); }
    else { xr[l] = -12345.0f; dn[l] = -12345.0f; }
}

void test_warp_reductions() {
    if (skip("warp_reductions")) return;
    const auto cases = make_vec_cases();
    with_pow2<1, 2, 4, 8, 16, 32>([&](auto wc) {
        constexpr int W = decltype(wc)::value;
        Dev<float> ds(32), dm(32), dxr(32), ddn(32), dmx(32);
        for (const auto& c : cases) {
            if (c.x.size() > 32) continue;
            std::vector<float> s(32, 0.0f), m(32, -kInf);
            for (std::size_t i = 0; i < c.x.size(); ++i) s[i] = m[i] = c.x[i];
            CK(cudaMemcpy(ds.p, s.data(), 128, cudaMemcpyHostToDevice));
            CK(cudaMemcpy(dm.p, m.data(), 128, cudaMemcpyHostToDevice));
            k_warp<W><<<1, 32>>>(ds.p, dm.p, dxr.p, ddn.p, dmx.p);
            sync();
            const auto xr = dxr.get(), dn = ddn.get(), mx = dmx.get();
            const auto txr = tree_xor(s, W), tdn = tree_down(s, W);
            for (int g = 0; g < 32; g += W) {
                const auto e = expect_sum(std::vector<float>(s.begin() + g, s.begin() + g + W), c.exact);
                float want_max = m[g];
                for (int l = g; l < g + W; ++l) want_max = maxnum(want_max, m[l]);
                std::string why, tag = c.name + " W" + std::to_string(W) + " group" + std::to_string(g);
                for (int l = g; l < g + W; ++l) {
                    check(sum_ok(e, xr[l], why), c.exact ? "EXACT" : "BOUND", "warp_sum " + tag + " lane" + std::to_string(l), why);
                    check(value_eq(mx[l], want_max), "EXACT", "warp_max " + tag + " lane" + std::to_string(l), "got " + std::to_string(mx[l]) + " want " + std::to_string(want_max));
                    if (g_pinned) check(same_bits_or_nan(xr[l], txr[l]), "PINNED", "warp_sum order " + tag, hex(bits_of(xr[l])) + " vs " + hex(bits_of(txr[l])));
                }
                check(sum_ok(e, dn[g], why), c.exact ? "EXACT" : "BOUND", "warp_reduce_sum " + tag, why);
                if (g_pinned) check(same_bits_or_nan(dn[g], tdn[g]), "PINNED", "warp_reduce_sum order " + tag, hex(bits_of(dn[g])) + " vs " + hex(bits_of(tdn[g])));
            }
        }
    });
    // partial masks: lanes 0..7 only, and the interleaved groups 8..15 / 24..31
    auto partial = [&](auto kernel, unsigned mask) {
        std::vector<float> s(32);
        for (int l = 0; l < 32; ++l) s[l] = float(l * l + 1);
        Dev<float> ds(s), dxr(32), ddn(32);
        kernel<<<1, 32>>>(ds.p, dxr.p, ddn.p);
        sync();
        const auto xr = dxr.get(), dn = ddn.get();
        for (int g = 0; g < 32; g += 8) {
            if (!((mask >> g) & 1u)) continue;
            float want = 0;
            for (int l = g; l < g + 8; ++l) want += s[l];
            for (int l = g; l < g + 8; ++l) check(xr[l] == want, "EXACT", "warp_sum<8> partial mask " + hex(mask) + " lane" + std::to_string(l));
            check(dn[g] == want, "EXACT", "warp_reduce_sum<8> partial mask " + hex(mask) + " lane" + std::to_string(g));
        }
    };
    if (!skip("warp_reductions_partial_mask")) { partial(k_warp_partial<0x000000ffu>, 0xffu); partial(k_warp_partial<0xff00ff00u>, 0xff00ff00u); }
}

// ============================================================================ block_reduce_sum / block_max_broadcast
template <int B> __global__ void k_block_sum(const float* x, int n, float* out) {
    __shared__ float sums[B / 32];
    float a = 0.0f;
    for (int i = threadIdx.x; i < n; i += B) a += x[i];
    const float r = block_reduce_sum<B>(a, sums);
    if (threadIdx.x == 0) out[0] = r; // only thread 0 owns the result
}
template <int B> __global__ void k_block_max(const float* x, int n, float* out) {
    const float r = block_max_broadcast<B>(threadIdx.x < n ? x[threadIdx.x] : -CUDART_INF_F);
    out[threadIdx.x] = r; // every thread receives the block maximum
}
void test_block_reductions() {
    if (skip("block_reductions")) return;
    const auto cases = make_vec_cases();
    with_pow2<32, 64, 128, 256, 512, 1024>([&](auto bc) {
        constexpr int B = decltype(bc)::value;
        Dev<float> dout(B);
        for (const auto& c : cases) {
            const int n = int(c.x.size());
            Dev<float> dx(c.x);
            k_block_sum<B><<<1, B>>>(dx.p, n, dout.p);
            sync();
            auto padded = c.x; padded.push_back(0.0f); // each thread starts from +0
            std::string why, tag = c.name + " B" + std::to_string(B);
            const float got = dout.get()[0];
            check(sum_ok(expect_sum(padded, c.exact), got, why), c.exact ? "EXACT" : "BOUND", "block_reduce_sum " + tag, why);
            if (g_pinned) check(same_bits_or_nan(got, model_block_sum(c.x, B)), "PINNED", "block_reduce_sum order " + tag, hex(bits_of(got)) + " vs " + hex(bits_of(model_block_sum(c.x, B))));
            if (n <= B) {
                k_block_max<B><<<1, B>>>(dx.p, n, dout.p);
                sync();
                float want = -kInf; bool all_nan = true;
                for (float f : c.x) { want = maxnum(want, f); all_nan = all_nan && std::isnan(f); }
                if (all_nan) continue; // all-NaN block: NaN or -Inf depending on B (identity padding); not asserted
                const auto r = dout.get();
                for (int t = 0; t < B; ++t) check(value_eq(r[t], want), "EXACT", "block_max_broadcast " + tag + " thread" + std::to_string(t), "got " + std::to_string(r[t]) + " want " + std::to_string(want));
            }
        }
    });
}

// ============================================================================ argmax
__global__ void k_warp_argmax(const float* v, const std::int32_t* id, int n, float* ov, std::int32_t* oi) {
    float x = threadIdx.x < n ? v[threadIdx.x] : -CUDART_INF_F;
    std::int32_t i = threadIdx.x < n ? id[threadIdx.x] : INT_MAX;
    warp_reduce_argmax(x, i);
    if (threadIdx.x == 0) { ov[0] = x; oi[0] = i; }
}
__global__ void k_block_argmax(const float* v, const std::int32_t* id, int n, float* ov, std::int32_t* oi) {
    float x = threadIdx.x < n ? v[threadIdx.x] : -CUDART_INF_F;
    std::int32_t i = threadIdx.x < n ? id[threadIdx.x] : INT_MAX;
    block_reduce_argmax(x, i);
    if (threadIdx.x == 0) { ov[0] = x; oi[0] = i; }
}
void test_argmax() {
    if (skip("argmax")) return;
    Dev<float> ov(1); Dev<std::int32_t> oi(1);
    for (const auto& c : make_arg_cases()) {
        const int n = int(c.v.size());
        Dev<float> dv(c.v); Dev<std::int32_t> di(c.id);
        auto verify = [&](const char* what, VI got, int block) {
            std::string tag = std::string(what) + " " + c.name + " B" + std::to_string(block);
            if (!c.has_nan) {
                const int b = ref_argmax(c, n);
                check(got.i == c.id[b] && bits_of(got.v) == bits_of(c.v[b]), "EXACT", tag, "got (" + std::to_string(got.v) + "," + std::to_string(got.i) + ") want (" + std::to_string(c.v[b]) + "," + std::to_string(c.id[b]) + ")");
            } else if (g_pinned) { // NaN is outside the finite score/id contract: characterize the tree only
                const VI m = block == 32 ? model_warp_argmax([&] { std::vector<VI> l(32, VI{-kInf, INT_MAX}); for (int t = 0; t < n; ++t) l[t] = {c.v[t], c.id[t]}; return l; }()) : model_block_argmax(c, block, n);
                check(same_bits_or_nan(got.v, m.v) && got.i == m.i, "PINNED", tag + " (NaN)", "got (" + std::to_string(got.v) + "," + std::to_string(got.i) + ") want (" + std::to_string(m.v) + "," + std::to_string(m.i) + ")");
            }
        };
        if (n <= 32) { k_warp_argmax<<<1, 32>>>(dv.p, di.p, n, ov.p, oi.p); sync(); verify("warp_reduce_argmax", {ov.get()[0], oi.get()[0]}, 32); }
        for (int B : {32, 64, 96, 160, 256, 480, 512}) {
            if (n > B) continue;
            k_block_argmax<<<1, B>>>(dv.p, di.p, n, ov.p, oi.p); sync();
            verify("block_reduce_argmax", {ov.get()[0], oi.get()[0]}, B);
        }
    }
}

// ============================================================================ order keys
__global__ void k_keys(const float* v, const std::int32_t* id, int n, unsigned long long* key, float* score, std::int32_t* rid, unsigned* ordered) {
    const int t = blockIdx.x * blockDim.x + threadIdx.x;
    if (t >= n) return;
    key[t] = score_id_order_key(v[t], id[t]);
    score[t] = score_from_order_key(key[t]);
    rid[t] = id_from_order_key(key[t]);
    ordered[t] = ordered_score_bits(v[t]);
}
__global__ void k_better(const float* v, const std::int32_t* id, int n, unsigned char* out) {
    const int a = blockIdx.x, b = threadIdx.x;
    if (b < n) out[a * n + b] = score_id_better(v[a], id[a], v[b], id[b]);
}
__global__ void k_max_key(const unsigned long long* keys, int n, unsigned long long* out) {
    __shared__ unsigned long long warp_keys[32];
    const unsigned long long r = block_reduce_max_key(threadIdx.x < n ? keys[threadIdx.x] : 0ull, warp_keys);
    if (threadIdx.x == 0) out[0] = r;
}
void test_order_keys() {
    if (skip("order_keys")) return;
    const float fmin_ = 1.17549435e-38f, dmin = float_of(1), fmax_ = 3.40282347e38f;
    std::vector<float> scores{-kInf, -fmax_, -1e10f, -2.0f, -1.0f, -fmin_, -dmin, -0.0f, 0.0f, dmin, fmin_, 1e-20f, 0.5f, 1.0f, float_of(0x3f800001u), 2.0f, 1e10f, fmax_, kInf};
    std::vector<std::int32_t> ids{0, 1, 2, 255, 256, 65535, 1 << 24, INT_MAX - 2, INT_MAX - 1, INT_MAX};
    std::vector<float> v; std::vector<std::int32_t> id;
    for (float s : scores) for (auto i : ids) { v.push_back(s); id.push_back(i); }
    const int n = int(v.size());
    Dev<float> dv(v), dsc(n); Dev<std::int32_t> di(id), drid(n); Dev<unsigned long long> dk(n); Dev<unsigned> dor(n); Dev<unsigned char> db(std::size_t(n) * n);
    k_keys<<<(n + 127) / 128, 128>>>(dv.p, di.p, n, dk.p, dsc.p, drid.p, dor.p);
    k_better<<<n, 256>>>(dv.p, di.p, n, db.p);
    sync();
    const auto key = dk.get(); const auto sc = dsc.get(); const auto rid = drid.get(); const auto better = db.get(); const auto ord = dor.get();
    auto canon = [](float f) { return f == 0.0f ? 0.0f : f; };
    for (int a = 0; a < n; ++a) {
        const bool sent_a = id[a] == INT_MAX;
        const std::string ta = "(" + std::to_string(v[a]) + "," + std::to_string(id[a]) + ")";
        check(rid[a] == id[a], "EXACT", "id_from_order_key roundtrip " + ta, std::to_string(rid[a]));
        if (!sent_a) check(bits_of(sc[a]) == bits_of(canon(v[a])), "EXACT", "score_from_order_key roundtrip " + ta, hex(bits_of(sc[a])));
        if (sent_a) check(key[a] == 0, "EXACT", "sentinel id INT_MAX gives key 0 " + ta);
        else check(key[a] != 0, "EXACT", "valid pair has nonzero key " + ta);
        for (int b = 0; b < n; ++b) {
            check(better[a * n + b] == (ref_better(v[a], id[a], v[b], id[b]) ? 1 : 0), "EXACT", "score_id_better " + ta + " vs (" + std::to_string(v[b]) + "," + std::to_string(id[b]) + ")");
            const bool sent_b = id[b] == INT_MAX;
            int want = sent_a || sent_b ? (sent_a == sent_b ? 0 : (sent_b ? 1 : -1))
                                        : (ref_better(v[a], id[a], v[b], id[b]) ? 1 : ref_better(v[b], id[b], v[a], id[a]) ? -1 : 0);
            const int got = key[a] > key[b] ? 1 : key[a] < key[b] ? -1 : 0;
            check(got == want, "EXACT", "order key compare " + ta + " vs (" + std::to_string(v[b]) + "," + std::to_string(id[b]) + ")", "got " + std::to_string(got) + " want " + std::to_string(want));
        }
    }
    for (int a = 0; a + 1 < int(scores.size()); ++a) // ordered_score_bits is strictly monotonic over distinct values
        if (canon(scores[a]) != canon(scores[a + 1])) check(ord[a * 10] < ord[(a + 1) * 10], "EXACT", "ordered_score_bits monotonic " + std::to_string(scores[a]));
    // block_reduce_max_key: keys of random pairs, thread >= n holds the reserved key 0
    Rng rng(99);
    Dev<unsigned long long> dout(1);
    for (int B : {32, 64, 96, 128, 224, 1024}) for (int cnt : {1, 31, 33, B}) {
        if (cnt > B) continue;
        std::vector<unsigned long long> k(cnt);
        for (auto& x : k) x = rng.next() % 4 == 0 ? 0ull : rng.next();
        Dev<unsigned long long> dkk(k);
        k_max_key<<<1, B>>>(dkk.p, cnt, dout.p); sync();
        unsigned long long want = 0; for (auto x : k) want = std::max(want, x);
        check(dout.get()[0] == want, "EXACT", "block_reduce_max_key B" + std::to_string(B) + " n" + std::to_string(cnt), hex(dout.get()[0]) + " vs " + hex(want));
    }
}

// ============================================================================ Welford
__global__ void k_wel_seq(const float* x, int n, WelfordMoments* out) {
    WelfordMoments m{0, 0, 0}; for (int i = 0; i < n; ++i) welford_update(m, x[i]); out[0] = m;
}
__global__ void k_wel_merge(const float* x, int n, WelfordMoments* out) {
    WelfordMoments a{0, 0, 0}, b{0, 0, 0}, empty{123.0f, 456.0f, 0};
    for (int i = 0; i < n / 2; ++i) welford_update(a, x[i]);
    for (int i = n / 2; i < n; ++i) welford_update(b, x[i]);
    out[0] = welford_merge(a, b);
    out[1] = welford_merge(a, empty); // count==0 operand returns the other one unchanged
    out[2] = welford_merge(empty, b);
    out[3] = a; out[4] = b;
}
__global__ void k_wel_warp(const float* x, const int* count, const int* offset, WelfordMoments* out) {
    WelfordMoments m{0, 0, 0};
    for (int i = 0; i < count[threadIdx.x]; ++i) welford_update(m, x[offset[threadIdx.x] + i]);
    m = warp_reduce_moments(m);
    if (threadIdx.x == 0) out[0] = m;
}
void test_welford() {
    if (skip("welford")) return;
    Dev<WelfordMoments> dout(5);
    for (const auto& c : make_moment_cases()) {
        const int n = int(c.x.size());
        const MomRef r = ref_moments(c.x);
        Dev<float> dx(c.x);
        std::string why;
        k_wel_seq<<<1, 1>>>(dx.p, n, dout.p); sync();
        auto o = dout.get()[0];
        check(moments_ok(r, o.mean, o.m2, o.count, why), "BOUND", "welford_update " + c.name, why);
        k_wel_merge<<<1, 1>>>(dx.p, n, dout.p); sync();
        const auto m = dout.get();
        check(moments_ok(r, m[0].mean, m[0].m2, m[0].count, why), "BOUND", "welford_merge halves " + c.name, why);
        check(std::memcmp(&m[1], &m[3], sizeof(WelfordMoments)) == 0 || m[3].count != m[1].count, "EXACT", "welford_merge(a, empty) == a " + c.name);
        check(m[1].count == m[3].count && bits_of(m[1].mean) == bits_of(m[3].mean) && bits_of(m[1].m2) == bits_of(m[3].m2), "EXACT", "welford_merge(a, empty) == a bits " + c.name);
        check(m[2].count == m[4].count && bits_of(m[2].mean) == bits_of(m[4].mean) && bits_of(m[2].m2) == bits_of(m[4].m2), "EXACT", "welford_merge(empty, b) == b bits " + c.name);
        for (int mode = 0; mode < 4; ++mode) {
            const auto cnt = split_counts(n, mode, 5000 + n);
            std::vector<int> off(32, 0);
            for (int l = 1; l < 32; ++l) off[l] = off[l - 1] + cnt[l - 1];
            Dev<int> dc(cnt), dof(off);
            k_wel_warp<<<1, 32>>>(dx.p, dc.p, dof.p, dout.p); sync();
            o = dout.get()[0];
            check(moments_ok(r, o.mean, o.m2, o.count, why), "BOUND", "warp_reduce_moments mode" + std::to_string(mode) + " " + c.name, why);
        }
    }
    // all lanes empty: the result stays {0, 0, 0}
    std::vector<int> z(32, 0); Dev<int> dz(z); Dev<float> dx(std::vector<float>{1.0f});
    k_wel_warp<<<1, 32>>>(dx.p, dz.p, dz.p, dout.p); sync();
    const auto o = dout.get()[0];
    check(o.count == 0 && o.mean == 0 && o.m2 == 0, "EXACT", "warp_reduce_moments all empty");
}

// ============================================================================ raw warp primitives
template <class T> struct Lane { static T make(int l); static unsigned long long raw(T v) { unsigned long long r = 0; std::memcpy(&r, &v, sizeof(T)); return r; } };
template <> int Lane<int>::make(int l) { return 1000 + 7 * l - (l & 1 ? 5000 : 0); }
template <> float Lane<float>::make(int l) { return 0.25f + 1.5f * float(l); }
template <> unsigned long long Lane<unsigned long long>::make(int l) { return (0x8000000000000000ull >> (l & 7)) | (static_cast<unsigned long long>(l) << 33) | (0xabcd0000u + l); }

// mode 0 idx, 1 xor, 2 down, 3 up
template <class T> __global__ void k_shfl(const T* in, int mode, int arg, int width, T* out) {
    const T v = in[threadIdx.x];
    out[threadIdx.x] = mode == 0 ? __shfl_sync(0xffffffffu, v, arg, width) : mode == 1 ? __shfl_xor_sync(0xffffffffu, v, arg, width)
                       : mode == 2 ? __shfl_down_sync(0xffffffffu, v, arg, width) : __shfl_up_sync(0xffffffffu, v, arg, width);
}
int shfl_src(int mode, int lane, int arg, int width) { // documented CUDA semantics; out of segment keeps own value
    const int base = lane / width * width, pos = lane - base;
    switch (mode) {
    case 0: return base + arg % width;
    case 1: return base + (pos ^ arg);
    case 2: return pos + arg < width ? lane + arg : lane;
    default: return pos - arg >= 0 ? lane - arg : lane;
    }
}
template <class T> void test_shfl_type(const char* tname) {
    std::vector<T> in(32);
    for (int l = 0; l < 32; ++l) in[l] = Lane<T>::make(l);
    Dev<T> din(in), dout(32);
    for (int width : {2, 4, 8, 16, 32}) for (int mode = 0; mode < 4; ++mode) {
        std::vector<int> args;
        if (mode == 0) args = {0, 1, 5, width - 1, width + 3};
        else if (mode == 1) for (int m = 1; m < width; m <<= 1) args.push_back(m), args.push_back(width - 1);
        else args = {0, 1, 2, 3, 5, width / 2, width - 1, width};
        for (int arg : args) {
            k_shfl<T><<<1, 32>>>(din.p, mode, arg, width, dout.p); sync();
            const auto out = dout.get();
            for (int l = 0; l < 32; ++l) {
                const int src = shfl_src(mode, l, arg, width);
                check(Lane<T>::raw(out[l]) == Lane<T>::raw(in[src]), "EXACT", std::string("__shfl") + (mode == 0 ? "" : mode == 1 ? "_xor" : mode == 2 ? "_down" : "_up") + "_sync<" + tname + "> width" + std::to_string(width) + " arg" + std::to_string(arg) + " lane" + std::to_string(l), "from lane " + std::to_string(src));
            }
        }
    }
}
void test_shfl() {
    if (skip("shfl")) return;
    test_shfl_type<int>("int"); test_shfl_type<float>("float"); test_shfl_type<unsigned long long>("u64");
}

// Participants = lanes in `mask`; pred bit l from `preds`.
__global__ void k_vote(unsigned mask, unsigned preds, unsigned long long* ballot, unsigned* rank, int* first, unsigned* active, int* sync_ok) {
    __shared__ int box[32];
    const int l = threadIdx.x;
    ballot[l] = ~0ull; rank[l] = ~0u; first[l] = -99; active[l] = ~0u; sync_ok[l] = -1;
    box[l] = -1;
    __syncwarp();
    if ((mask >> l) & 1u) {
        const unsigned long long b = __ballot_sync(mask, (preds >> l) & 1u);
        ballot[l] = b;
        rank[l]   = __popc(static_cast<unsigned>(b) & ((1u << l) - 1u));
        first[l]  = __ffs(static_cast<unsigned>(b)) - 1;
        active[l] = __activemask();
        box[l] = l * 3;
        __syncwarp(mask);
        sync_ok[l] = box[(l + 1) % 32] == -1 ? 0 : 1; // neighbour is written iff it is in the mask
    }
}
__global__ void k_bits(const unsigned* in, int n, unsigned* popc, int* ffs) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) { popc[i] = __popc(in[i]); ffs[i] = __ffs(static_cast<int>(in[i])); }
}
__global__ void k_wave(unsigned long long* hi_ballot, unsigned long long* odd_ballot, unsigned* src0, int* wsize, unsigned long long* full_popc) {
    const int t = threadIdx.x;
    hi_ballot[t]  = __ballot_sync(0xffffffffu, t >= 32);
    odd_ballot[t] = __ballot_sync(0xffffffffu, t & 1);
    src0[t]       = __shfl_sync(0xffffffffu, t, 0); // default width = hardware warpSize
    wsize[t]      = warpSize;
    full_popc[t]  = __popc(static_cast<unsigned>(__ballot_sync(0xffffffffu, true)));
}
unsigned ref_ballot(unsigned mask, unsigned preds) { return preds & mask; }
int ref_popc(unsigned v) { int c = 0; for (; v; v &= v - 1) ++c; return c; }
int ref_ffs(unsigned v) { if (!v) return 0; int i = 1; while (!(v & 1)) { v >>= 1; ++i; } return i; }
void test_vote_and_bits() {
    if (!skip("vote")) {
        Rng rng(2024);
        std::vector<unsigned> masks{0xffffffffu, 0x0000ffffu, 0xffff0000u, 0x80000001u, 0x00000001u, 0x80000000u, 0xaaaaaaaau, 0x55555555u, 0x0f0f0f0fu, 0x00ff00ffu};
        for (int i = 0; i < 12; ++i) masks.push_back(unsigned(rng.next()) | 1u);
        std::vector<unsigned> preds{0u, 0xffffffffu, 0x80000000u, 1u, 0xaaaaaaaau};
        for (int i = 0; i < 8; ++i) preds.push_back(unsigned(rng.next()));
        Dev<unsigned long long> db(32); Dev<unsigned> dr(32), da(32); Dev<int> df(32), ds(32);
        for (unsigned mask : masks) for (unsigned p : preds) {
            k_vote<<<1, 32>>>(mask, p, db.p, dr.p, df.p, da.p, ds.p); sync();
            const auto b = db.get(); const auto r = dr.get(); const auto f = df.get(); const auto a = da.get(); const auto s = ds.get();
            const unsigned want = ref_ballot(mask, p);
            for (int l = 0; l < 32; ++l) {
                if (!((mask >> l) & 1u)) continue;
                const std::string tag = "mask " + hex(mask) + " preds " + hex(p) + " lane" + std::to_string(l);
                check(b[l] == want, "EXACT", "__ballot_sync (32-bit result, zero upper bits) " + tag, hex(b[l]) + " vs " + hex(want));
                check(r[l] == unsigned(ref_popc(want & ((1u << l) - 1u))), "EXACT", "__popc rank " + tag, std::to_string(r[l]));
                check(f[l] == ref_ffs(want) - 1, "EXACT", "__ffs first " + tag, std::to_string(f[l]));
                check(a[l] == mask, "EXACT", "__activemask inside the masked branch " + tag, hex(a[l]));
                check(s[l] == (((mask >> ((l + 1) % 32)) & 1u) ? 1 : 0), "EXACT", "__syncwarp(mask) orders the shared writes " + tag, std::to_string(s[l]));
            }
        }
    }
    if (!skip("popc_ffs")) {
        Rng rng(77);
        std::vector<unsigned> in{0u, 1u, 0x80000000u, 0xffffffffu, 0x7fffffffu, 0xfffffffeu, 0x00010000u, 0xaaaaaaaau, 0x55555555u};
        for (int b = 0; b < 32; ++b) in.push_back(1u << b), in.push_back((0xffffffffu << b) ^ 0x1u);
        for (int i = 0; i < 2000; ++i) in.push_back(unsigned(rng.next()) >> rng.uniform(0, 31));
        Dev<unsigned> din(in), dp(in.size()); Dev<int> df(in.size());
        k_bits<<<int(in.size() + 255) / 256, 256>>>(din.p, int(in.size()), dp.p, df.p); sync();
        const auto p = dp.get(); const auto f = df.get();
        for (std::size_t i = 0; i < in.size(); ++i) {
            check(int(p[i]) == ref_popc(in[i]), "EXACT", "__popc " + hex(in[i]), std::to_string(p[i]));
            check(f[i] == ref_ffs(in[i]), "EXACT", "__ffs " + hex(in[i]), std::to_string(f[i]));
        }
    }
    if (!skip("wave32")) { // wave64 detector: two 32-lane warps in one 64-thread block must have distinct masks
        Dev<unsigned long long> dh(64), dod(64), dpc(64); Dev<unsigned> ds(64); Dev<int> dw(64);
        k_wave<<<1, 64>>>(dh.p, dod.p, ds.p, dw.p, dpc.p); sync();
        const auto h = dh.get(), od = dod.get(), pc = dpc.get(); const auto s = ds.get(); const auto w = dw.get();
        for (int t = 0; t < 64; ++t) {
            const std::string tag = "thread" + std::to_string(t);
            check(w[t] == 32, "EXACT", "warpSize == 32 " + tag, std::to_string(w[t]));
            check(h[t] == (t >= 32 ? 0xffffffffull : 0ull), "EXACT", "__ballot_sync(full, t>=32) is a per-warp 32-bit mask " + tag, hex(h[t]));
            check(od[t] == 0xaaaaaaaaull, "EXACT", "__ballot_sync(full, odd) " + tag, hex(od[t]));
            check(s[t] == unsigned(t & ~31), "EXACT", "__shfl_sync(full, tid, 0) stays inside the 32-lane warp " + tag, std::to_string(s[t]));
            check(pc[t] == 32, "EXACT", "popc(ballot(true)) == 32 " + tag, std::to_string(pc[t]));
        }
    }
}

// ============================================================================ __reduce_*_sync (sm_80+ / HIP shim)
#if !defined(__CUDA_ARCH__) || __CUDA_ARCH__ >= 800
__global__ void k_reduce(unsigned mask, const unsigned* u, const int* s, unsigned* out) {
    const int l = threadIdx.x;
    if (!((mask >> l) & 1u)) return;
    unsigned r[9] = {__reduce_add_sync(mask, u[l]), __reduce_min_sync(mask, u[l]), __reduce_max_sync(mask, u[l]), __reduce_and_sync(mask, u[l]),
                     __reduce_or_sync(mask, u[l]), __reduce_xor_sync(mask, u[l]), unsigned(__reduce_add_sync(mask, s[l])),
                     unsigned(__reduce_min_sync(mask, s[l])), unsigned(__reduce_max_sync(mask, s[l]))};
    for (int k = 0; k < 9; ++k) out[l * 9 + k] = r[k];
}
#endif
void test_reduce_sync() {
    if (skip("reduce_sync")) return;
#if !defined(__CUDA_ARCH__) || __CUDA_ARCH__ >= 800
    Rng rng(31337);
    std::vector<unsigned> u(32); std::vector<int> s(32);
    for (int l = 0; l < 32; ++l) { u[l] = l % 3 == 0 ? 0xffffffffu - unsigned(l) : unsigned(rng.next()); s[l] = int(rng.next() % 2000001) - 1000000; }
    Dev<unsigned> du(u), dout(32 * 9); Dev<int> ds(s);
    for (unsigned mask : {0xffffffffu, 0x0000ffffu, 0x80000001u, 0x00000001u, 0xaaaaaaaau, 0x00ff00f0u}) {
        k_reduce<<<1, 32>>>(mask, du.p, ds.p, dout.p); sync();
        const auto o = dout.get();
        unsigned add = 0, mn = ~0u, mx = 0, an = ~0u, orr = 0, xr = 0; long long sadd = 0; int smn = INT_MAX, smx = INT_MIN;
        for (int l = 0; l < 32; ++l) if ((mask >> l) & 1u) { add += u[l]; mn = std::min(mn, u[l]); mx = std::max(mx, u[l]); an &= u[l]; orr |= u[l]; xr ^= u[l]; sadd += s[l]; smn = std::min(smn, s[l]); smx = std::max(smx, s[l]); }
        const unsigned want[9] = {add, mn, mx, an, orr, xr, unsigned(int(sadd)), unsigned(smn), unsigned(smx)};
        const char* nm[9] = {"add_u", "min_u", "max_u", "and", "or", "xor", "add_s", "min_s", "max_s"};
        for (int l = 0; l < 32; ++l) if ((mask >> l) & 1u) for (int k = 0; k < 9; ++k)
            check(o[l * 9 + k] == want[k], "EXACT", std::string("__reduce_") + nm[k] + "_sync mask " + hex(mask) + " lane" + std::to_string(l), hex(o[l * 9 + k]) + " vs " + hex(want[k]));
    }
#endif
}

// ============================================================================ atomics
constexpr int kBlocks = 4, kThreads = 256, kBins = 7;
__global__ void k_atomic_int(int* counter, int* olds, int* hist, int* shared_out, float* fcounter, float* folds, float* fhist) {
    const int t = blockIdx.x * blockDim.x + threadIdx.x;
    olds[t]  = atomicAdd(counter, 1);
    folds[t] = atomicAdd(fcounter, 1.0f);
    atomicAdd(hist + t % kBins, t % 5 - 2);
    atomicAdd(fhist + t % kBins, float(t % 5 - 2));
    __shared__ int acc; __shared__ float facc;
    if (threadIdx.x == 0) { acc = 0; facc = 0; }
    __syncthreads();
    atomicAdd(&acc, int(threadIdx.x) % 9 - 4);
    atomicAdd(&facc, float(int(threadIdx.x) % 9 - 4));
    __syncthreads();
    if (threadIdx.x == 0) { shared_out[blockIdx.x] = acc; shared_out[kBlocks + blockIdx.x] = int(facc); }
}
__global__ void k_atomic_special(float* out) { // 3 threads per target; Inf/NaN propagate as in IEEE addition
    const int t = threadIdx.x, g = t / 3, k = t % 3;
    static const float v[5][3] = {{INFINITY, 1.0f, 2.0f}, {INFINITY, -INFINITY, 1.0f}, {NAN, 1.0f, 2.0f}, {-INFINITY, -4.0f, 1.0f}, {0.0f, -0.0f, -0.0f}};
    atomicAdd(out + g, v[g][k]);
}
__global__ void k_cas(unsigned* counter, unsigned long long* wide, unsigned* single) {
    unsigned old = *counter, assumed;
    do { assumed = old; old = atomicCAS(counter, assumed, assumed + 1u); } while (old != assumed);
    unsigned long long o = *wide, a;
    do { a = o; o = atomicCAS(wide, a, a + (1ull << 33)); } while (o != a);
    if (blockIdx.x == 0 && threadIdx.x == 0) { // single-thread semantics: returns the old value, swaps only on match
        single[1] = atomicCAS(single, 5u, 9u);
        single[2] = atomicCAS(single, 5u, 11u);
    }
}
void test_atomics() {
    if (skip("atomics")) return;
    const int n = kBlocks * kThreads;
    Dev<int> counter(1), olds(n), hist(kBins), shared_out(2 * kBlocks); Dev<float> fcounter(1), folds(n), fhist(kBins);
    k_atomic_int<<<kBlocks, kThreads>>>(counter.p, olds.p, hist.p, shared_out.p, fcounter.p, folds.p, fhist.p); sync();
    check(counter.get()[0] == n && fcounter.get()[0] == float(n), "EXACT", "atomicAdd int/float total");
    auto o = olds.get(); auto fo = folds.get();
    std::vector<int> seen(n, 0), fseen(n, 0); bool ok = true, fok = true;
    for (int t = 0; t < n; ++t) { ok = ok && o[t] >= 0 && o[t] < n && !seen[o[t]]++; fok = fok && fo[t] >= 0 && fo[t] < n && fo[t] == std::floor(fo[t]) && !fseen[int(fo[t])]++; }
    check(ok, "EXACT", "atomicAdd(int) returned old values are a permutation of 0..N-1");
    check(fok, "EXACT", "atomicAdd(float) returned old values are a permutation of 0..N-1");
    const auto h = hist.get(); const auto fh = fhist.get();
    for (int b = 0; b < kBins; ++b) {
        int want = 0; for (int t = b; t < n; t += kBins) want += t % 5 - 2;
        check(h[b] == want && fh[b] == float(want), "EXACT", "atomicAdd histogram bin " + std::to_string(b), std::to_string(h[b]) + "/" + std::to_string(fh[b]) + " vs " + std::to_string(want));
    }
    const auto so = shared_out.get();
    int want_block = 0; for (int t = 0; t < kThreads; ++t) want_block += t % 9 - 4;
    for (int b = 0; b < kBlocks; ++b) check(so[b] == want_block && so[kBlocks + b] == want_block, "EXACT", "shared-memory atomicAdd int/float block " + std::to_string(b));
    Dev<float> sp(5); k_atomic_special<<<1, 15>>>(sp.p); sync();
    const auto r = sp.get();
    check(r[0] == kInf && std::isnan(r[1]) && std::isnan(r[2]) && r[3] == -kInf && r[4] == 0.0f,
          "EXACT", "atomicAdd(float) with +-Inf/NaN follows IEEE (zero-sign of the last target is not asserted)");
    Dev<unsigned> cas(1), single(std::vector<unsigned>{5u, 0u, 0u}); Dev<unsigned long long> wide(1);
    k_cas<<<kBlocks, kThreads>>>(cas.p, wide.p, single.p); sync();
    check(cas.get()[0] == unsigned(n), "EXACT", "atomicCAS(unsigned) increment loop");
    check(wide.get()[0] == (unsigned long long)n << 33, "EXACT", "atomicCAS(unsigned long long) increment loop");
    const auto sg = single.get();
    check(sg[0] == 9u && sg[1] == 5u && sg[2] == 9u, "EXACT", "atomicCAS returns old value and swaps only on match", std::to_string(sg[0]) + "," + std::to_string(sg[1]) + "," + std::to_string(sg[2]));
}

// ============================================================================ memory.cuh
__global__ void k_memory(const float* src, unsigned* out) {
    const int t = threadIdx.x;
    const uint4 a = load_vec<uint4>(src + 4 * t), b = load_ldg<uint4>(src + 4 * t);
    const uint2 c = load_vec<uint2>(src + 2 * t), d = load_ldg<uint2>(src + 2 * t);
    const unsigned e = load_vec<unsigned>(src + t), f = load_ldg<unsigned>(src + t);
    const unsigned short g = load_vec<unsigned short>(src + t), h = load_ldg<unsigned short>(src + t);
    const unsigned char i = load_vec<unsigned char>(src + t), j = load_ldg<unsigned char>(src + t);
    unsigned* o = out + 24 * t;
    o[0] = a.x; o[1] = a.y; o[2] = a.z; o[3] = a.w; o[4] = b.x; o[5] = b.y; o[6] = b.z; o[7] = b.w;
    o[8] = c.x; o[9] = c.y; o[10] = d.x; o[11] = d.y; o[12] = e; o[13] = f; o[14] = g; o[15] = h; o[16] = i; o[17] = j;
    prefetch_l2(src, 4096, 0); prefetch_l2(src, 4096, 8192); prefetch_l2(nullptr, 0, 0); // must not trap or change data
    store_vec(reinterpret_cast<float*>(o + 18), make_uint2(0x11111111u, 0x22222222u)); // 8-byte store
    store_vec(o + 20, static_cast<unsigned short>(0xbeef));
}
void test_memory() {
    if (skip("memory")) return;
    std::vector<unsigned> w(256);
    for (int i = 0; i < 256; ++i) w[i] = 0x9e3779b9u * unsigned(i + 1) ^ 0xa5a5a5a5u;
    std::vector<float> src(256); std::memcpy(src.data(), w.data(), 1024);
    Dev<float> dsrc(src); Dev<unsigned> dout(24 * 64);
    k_memory<<<1, 64>>>(dsrc.p, dout.p); sync();
    const auto o = dout.get();
    const auto* bytes = reinterpret_cast<const unsigned char*>(w.data());
    for (int t = 0; t < 64; ++t) {
        const unsigned* r = &o[24 * t];
        const std::string tag = " thread" + std::to_string(t);
        for (int k = 0; k < 4; ++k) check(r[k] == w[4 * t + k] && r[4 + k] == w[4 * t + k], "EXACT", "load_vec/load_ldg<uint4>" + tag);
        check(r[8] == w[2 * t] && r[9] == w[2 * t + 1] && r[10] == r[8] && r[11] == r[9], "EXACT", "load_vec/load_ldg<uint2>" + tag);
        check(r[12] == w[t] && r[13] == w[t], "EXACT", "load_vec/load_ldg<unsigned>" + tag);
        unsigned short s; std::memcpy(&s, bytes + 4 * t, 2);
        check(r[14] == s && r[15] == s && r[16] == bytes[4 * t] && r[17] == bytes[4 * t], "EXACT", "load_vec/load_ldg 16/8-bit" + tag);
        check(r[18] == 0x11111111u && r[19] == 0x22222222u && (r[20] & 0xffffu) == 0xbeefu, "EXACT", "store_vec" + tag);
    }
    const auto after = dsrc.get();
    check(std::memcmp(after.data(), src.data(), 1024) == 0, "EXACT", "prefetch_l2 leaves data unchanged");
    for (int row = 0; row < 16; ++row) { // swizzle: permutation of the 8-element vectors, element order inside a vector kept
        std::vector<int> seen64(64, 0), seen128(128, 0);
        for (int col = 0; col < 64; ++col) { const int c = (col ^ ((row & 7) << 3)); check(c >= 0 && c < 64 && (col % 8) == (c % 8) && !seen64[c]++, "EXACT", "swizzle_xor_64 formula row" + std::to_string(row)); }
        for (int col = 0; col < 128; ++col) { const int c = (col ^ ((row & 7) << 4)); check(c >= 0 && c < 128 && (col % 16) == (c % 16) && !seen128[c]++, "EXACT", "swizzle_xor_128 formula row" + std::to_string(row)); }
    }
}
__global__ void k_swizzle(int* out) { // runs the real functions over rows 0..15 x cols 0..127
    const int row = blockIdx.x, col = threadIdx.x;
    out[row * 256 + col] = swizzle_xor_128(row, col);
    out[row * 256 + 128 + col] = col < 64 ? swizzle_xor_64(row, col) : -1;
}
void test_swizzle() {
    if (skip("swizzle")) return;
    Dev<int> d(16 * 256); k_swizzle<<<16, 128>>>(d.p); sync();
    const auto o = d.get();
    for (int row = 0; row < 16; ++row) for (int col = 0; col < 128; ++col) {
        check(o[row * 256 + col] == (col ^ ((row & 7) << 4)), "EXACT", "swizzle_xor_128 row" + std::to_string(row) + " col" + std::to_string(col));
        if (col < 64) check(o[row * 256 + 128 + col] == (col ^ ((row & 7) << 3)), "EXACT", "swizzle_xor_64 row" + std::to_string(row) + " col" + std::to_string(col));
    }
}

} // namespace

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--no-pinned")) g_pinned = false;
        else if (!std::strcmp(argv[i], "--filter") && i + 1 < argc) g_filter = argv[++i];
        else { std::fprintf(stderr, "usage: %s [--no-pinned] [--filter TEXT]\n", argv[0]); return 2; }
    }
    int dev = 0; CK(cudaGetDevice(&dev));
    cudaDeviceProp prop; CK(cudaGetDeviceProperties(&prop, dev));
    std::printf("device %s warpSize %d\n", prop.name, prop.warpSize);
    if (prop.warpSize != 32) { std::printf("FAIL [EXACT] host warpSize == 32 (wave32 required)\n"); return 1; }
    test_warp_reductions(); test_block_reductions(); test_argmax(); test_order_keys(); test_welford();
    test_shfl(); test_vote_and_bits(); test_reduce_sync(); test_atomics(); test_memory(); test_swizzle();
    std::printf("%d checks, %d failures%s\n", g_checks, g_fail, g_pinned ? "" : " (PINNED tier skipped)");
    return g_fail ? 1 : 0;
}
