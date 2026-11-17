// Host-only reference for tests/ops/common/test_common_primitives.cu (R97-09 oracle).
//
// Expected values come from mathematical definitions (exact summation, maxNum, lexicographic
// argmax, exact Welford moments), never from src/ops/common. Inputs are generated here by a
// self-contained PRNG so that the .cu test, the host dump (reference_dump.cpp) and the Python
// cross-check (crosscheck_reference.py) all see the same cases.
//
// Tiers used by the test:
//   EXACT  - contract: order independent. Data are exactly representable (every partial sum is
//            exact) or the result is a class (NaN, +Inf, -Inf); comparison is bit-exact (NaN: class).
//   BOUND  - contract: any summation order. |got - S| <= gamma_{n-1} * sum|x| with
//            gamma_k = k*u/(1-k*u), u = 2^-24 (Higham, Accuracy and Stability, thm 4.2/eq 4.4).
//   PINNED - characterization of the current shuffle trees (xor/down butterfly, lane 0 of each
//            group). Not a mathematical contract; fails if the port changes the add order
//            or the NaN behaviour of argmax. Waive with --no-pinned only with a written reason.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

namespace ninfer::test::common_ref {

inline std::uint32_t bits_of(float f) { std::uint32_t b; std::memcpy(&b, &f, 4); return b; }
inline float float_of(std::uint32_t b) { float f; std::memcpy(&f, &b, 4); return f; }
inline float fadd(float a, float b) { volatile float r = a + b; return r; } // strict binary32 add

inline std::uint64_t name_hash(const char* s) { // FNV-1a: stable across standard libraries
    std::uint64_t h = 1469598103934665603ull;
    for (; *s; ++s) h = (h ^ std::uint8_t(*s)) * 1099511628211ull;
    return h % 1000003;
}

struct Rng { // splitmix64
    std::uint64_t s;
    explicit Rng(std::uint64_t seed) : s(seed) {}
    std::uint64_t next() {
        std::uint64_t z = (s += 0x9e3779b97f4a7c15ull);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
        return z ^ (z >> 31);
    }
    int uniform(int lo, int hi) { return lo + int(next() % std::uint64_t(hi - lo + 1)); } // inclusive
    float unit() { return float(next() >> 40) / float(1 << 24); }                         // [0,1)
};

// Exact summation of doubles (Shewchuk partials, as Python math.fsum); finite inputs only.
inline double fsum(const std::vector<double>& v) {
    std::vector<double> p;
    for (double x : v) {
        std::size_t k = 0;
        for (std::size_t j = 0; j < p.size(); ++j) {
            double y = p[j];
            if (std::fabs(x) < std::fabs(y)) std::swap(x, y);
            const double hi = x + y, lo = y - (hi - x);
            if (lo != 0.0) p[k++] = lo;
            x = hi;
        }
        p.resize(k);
        p.push_back(x);
    }
    double s = 0;
    for (std::size_t j = p.size(); j-- > 0;) s += p[j];
    return s;
}

inline constexpr float kInf = std::numeric_limits<float>::infinity();
inline const float kNaN     = std::numeric_limits<float>::quiet_NaN();

// ---------------------------------------------------------------- float vector cases
struct VecCase {
    std::string name;
    std::vector<float> x;
    bool exact; // order independent: EXACT tier; otherwise BOUND + PINNED
};

inline const std::vector<int>& vec_sizes() {
    static const std::vector<int> v{1, 2, 3, 5, 7, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 129, 255, 257, 511, 1000, 1023, 1024};
    return v;
}

inline std::vector<VecCase> make_vec_cases() {
    std::vector<VecCase> out;
    for (int n : vec_sizes()) {
        auto add = [&](const char* kind, bool exact, auto gen) {
            Rng rng(0x1234567ull * unsigned(n) + name_hash(kind));
            VecCase c{std::string(kind) + "/n" + std::to_string(n), std::vector<float>(n), exact};
            for (int i = 0; i < n; ++i) c.x[i] = gen(rng, i);
            out.push_back(std::move(c));
        };
        add("ints", true, [](Rng& r, int) { return float(r.uniform(-1000, 1000)); });
        add("eighths", true, [](Rng& r, int) { return float(r.uniform(-4000, 4000)) / 8.0f; });
        add("ones", true, [](Rng&, int) { return 1.0f; });
        add("negzero", true, [](Rng&, int) { return -0.0f; });
        add("mixzero", true, [](Rng& r, int) { return (r.next() & 1) ? -0.0f : 0.0f; });
        add("subnormal", true, [](Rng& r, int) { return float_of(0x80000000u * (r.next() & 1) + unsigned(r.uniform(0, 50))); });
        add("nan_first", true, [n](Rng& r, int i) { return i == 0 ? kNaN : float(r.uniform(-9, 9)); });
        add("nan_mid", true, [n](Rng& r, int i) { return i == n / 2 ? kNaN : float(r.uniform(-9, 9)); });
        add("nan_last", true, [n](Rng& r, int i) { return i == n - 1 ? kNaN : float(r.uniform(-9, 9)); });
        add("all_nan", true, [](Rng&, int) { return kNaN; });
        add("inf_pos", true, [n](Rng& r, int i) { return i == n / 2 ? kInf : float(r.uniform(-9, 9)); });
        add("inf_neg", true, [n](Rng& r, int i) { return i == n - 1 ? -kInf : float(r.uniform(-9, 9)); });
        add("inf_cancel", true, [n](Rng& r, int i) { // +Inf and -Inf (n==1: only +Inf)
            return i == 0 ? kInf : (i == n - 1 ? -kInf : float(r.uniform(-9, 9))); });
        add("inf_same", true, [n](Rng& r, int i) { return (i == 0 || i == n - 1) ? kInf : float(r.uniform(-9, 9)); });
        add("inf_nan", true, [n](Rng& r, int i) { return i == 0 ? kInf : (i == n - 1 ? kNaN : float(r.uniform(-9, 9))); });
        add("wide", false, [](Rng& r, int) { // random exponent: result depends on the order
            return std::ldexp((r.next() & 1 ? -1.0f : 1.0f) * (1.0f + r.unit()), r.uniform(-30, 30)); });
        add("cancel", false, [](Rng&, int i) { // +-2^25 swallow the unit values in one order but not another
            return i % 3 == 0 ? 33554432.0f : (i % 3 == 1 ? -33554432.0f : 1.0f); });
    }
    return out;
}

// ---------------------------------------------------------------- sum / max reference
struct SumExpect {
    enum Kind { Finite, Nan, PosInf, NegInf } kind = Finite;
    double exact = 0, bound = 0;
    bool exact_data = false, neg_zero = false;
};

// `padded` is exactly what the participating threads contribute (identity +0 for padding).
inline SumExpect expect_sum(const std::vector<float>& padded, bool exact_data) {
    SumExpect e;
    e.exact_data = exact_data;
    bool nan = false, pinf = false, ninf = false, all_negzero = !padded.empty();
    std::vector<double> d, a;
    for (float f : padded) {
        all_negzero = all_negzero && bits_of(f) == 0x80000000u;
        if (std::isnan(f)) nan = true;
        else if (std::isinf(f)) (f > 0 ? pinf : ninf) = true;
        else { d.push_back(f); a.push_back(std::fabs(double(f))); }
    }
    if (nan || (pinf && ninf)) e.kind = SumExpect::Nan;
    else if (pinf) e.kind = SumExpect::PosInf;
    else if (ninf) e.kind = SumExpect::NegInf;
    e.exact = fsum(d);
    e.neg_zero = all_negzero;
    const double n = double(d.size()), u = std::ldexp(1.0, -24);
    e.bound = exact_data ? 0.0 : (n - 1) * u / (1 - (n - 1) * u) * fsum(a);
    if (exact_data) { // self-check: the claimed exactness really holds in any order
        if (double(float(e.exact)) != e.exact || fsum(a) * 8 >= 16777216.0) std::abort();
        for (double v : d) if (v * 8 != std::floor(v * 8) && std::fabs(v) > 1e-30) std::abort();
    }
    return e;
}

inline bool sum_ok(const SumExpect& e, float got, std::string& why) {
    const auto bad = [&](const std::string& m) { why = m + " got bits 0x" + std::to_string(bits_of(got)) + " value " + std::to_string(got); return false; };
    switch (e.kind) {
    case SumExpect::Nan: return std::isnan(got) ? true : bad("expected NaN");
    case SumExpect::PosInf: return got == kInf ? true : bad("expected +Inf");
    case SumExpect::NegInf: return got == -kInf ? true : bad("expected -Inf");
    case SumExpect::Finite: break;
    }
    if (!std::isfinite(got)) return bad("expected finite " + std::to_string(e.exact));
    if (e.exact_data) {
        const std::uint32_t want = e.exact == 0 ? (e.neg_zero ? 0x80000000u : 0u) : bits_of(float(e.exact));
        return bits_of(got) == want ? true : bad("expected exact bits " + std::to_string(want) + " (" + std::to_string(e.exact) + ")");
    }
    return std::fabs(double(got) - e.exact) <= e.bound ? true : bad("exceeds bound " + std::to_string(e.bound) + " vs exact " + std::to_string(e.exact));
}

inline float maxnum(float a, float b) { return std::isnan(a) ? b : std::isnan(b) ? a : (a > b ? a : b); }
inline bool value_eq(float a, float b) { return (std::isnan(a) && std::isnan(b)) || a == b; } // +-0 equal

// ---- PINNED models of the shuffle trees (lane values of 32 lanes; Width divides 32)
inline std::vector<float> tree_xor(std::vector<float> v, int W) {
    for (int o = W / 2; o > 0; o >>= 1) {
        auto n = v;
        for (int l = 0; l < 32; ++l) n[l] = fadd(v[l], v[l ^ o]);
        v = n;
    }
    return v;
}
inline std::vector<float> tree_down(std::vector<float> v, int W) {
    for (int o = W / 2; o > 0; o >>= 1) {
        auto n = v;
        for (int l = 0; l < 32; ++l) n[l] = fadd(v[l], (l % W + o < W) ? v[l + o] : v[l]);
        v = n;
    }
    return v;
}
// block_reduce_sum<B>: thread t accumulates x[t], x[t+B], ... from +0, then warp trees, thread 0.
inline float model_block_sum(const std::vector<float>& x, int B) {
    const int warps = B / 32;
    std::vector<float> part(B, 0.0f);
    for (std::size_t i = 0; i < x.size(); ++i) part[i % B] = fadd(part[i % B], x[i]);
    std::vector<float> w(32, 0.0f);
    for (int k = 0; k < warps; ++k) w[k] = tree_down(std::vector<float>(part.begin() + 32 * k, part.begin() + 32 * k + 32), 32)[0];
    return warps == 1 ? w[0] : tree_down(w, warps)[0]; // lanes >= warps already +0
}

// ---------------------------------------------------------------- argmax cases
struct ArgCase {
    std::string name;
    std::vector<float> v;
    std::vector<std::int32_t> id;
    bool has_nan; // outside the finite score/id contract: PINNED tier only
};
inline const std::vector<int>& arg_sizes() { static const std::vector<int> v{1, 2, 3, 31, 32, 33, 64, 100, 257, 480, 512}; return v; }

inline std::vector<ArgCase> make_arg_cases() {
    std::vector<ArgCase> out;
    for (int n : arg_sizes()) {
        for (int pat = 0; pat < 4; ++pat) { // index layout: identity / reversed / permutation / sparse big
            auto add = [&](const char* kind, bool nan, auto gen) {
                Rng rng(0xabcdull * unsigned(n) + 977 * unsigned(pat) + name_hash(kind));
                ArgCase c{std::string(kind) + "/ids" + std::to_string(pat) + "/n" + std::to_string(n), std::vector<float>(n), std::vector<std::int32_t>(n), nan};
                std::vector<int> perm(n);
                for (int i = 0; i < n; ++i) perm[i] = i;
                for (int i = n - 1; i > 0; --i) std::swap(perm[i], perm[rng.uniform(0, i)]);
                for (int i = 0; i < n; ++i) {
                    c.v[i]  = gen(rng, i);
                    c.id[i] = pat == 0 ? i : pat == 1 ? n - 1 - i : pat == 2 ? perm[i]
                                                                           : std::int32_t(2147483000 - 997 * perm[i] - 1);
                }
                out.push_back(std::move(c));
            };
            add("random", false, [](Rng& r, int) { return std::ldexp(r.unit() - 0.5f, r.uniform(-20, 20)); });
            add("all_equal", false, [](Rng&, int) { return 3.5f; });
            add("ties_max", false, [](Rng& r, int i) { return (i % 7 == 3) ? 9.0f : float(r.uniform(-5, 8)); });
            add("zero_tie", false, [](Rng& r, int i) { return (i % 2) ? -0.0f : 0.0f; });
            add("all_neginf", false, [](Rng&, int) { return -kInf; });
            add("inf_ties", false, [](Rng& r, int i) { return (i % 5 == 1) ? kInf : float(r.uniform(-5, 5)); });
            add("max_first", false, [](Rng& r, int i) { return i == 0 ? 100.0f : float(r.uniform(-5, 5)); });
            add("max_last", false, [n](Rng& r, int i) { return i == n - 1 ? 100.0f : float(r.uniform(-5, 5)); });
            add("nan_first", true, [](Rng& r, int i) { return i == 0 ? kNaN : float(r.uniform(-5, 5)); });
            add("nan_mid", true, [n](Rng& r, int i) { return i == n / 2 ? kNaN : float(r.uniform(-5, 5)); });
        }
    }
    return out;
}

inline int ref_argmax(const ArgCase& c, int n) { // larger value wins; ties -> lower id; +-0 equal
    int b = 0;
    for (int i = 1; i < n; ++i) if (c.v[i] > c.v[b] || (c.v[i] == c.v[b] && c.id[i] < c.id[b])) b = i;
    return b;
}
inline bool ref_better(float v, std::int32_t i, float bv, std::int32_t bi) { return v > bv || (v == bv && i < bi); }

struct VI { float v; std::int32_t i; };
inline VI model_warp_argmax(std::vector<VI> l) { // PINNED: shfl_down tree, lane 0
    for (int o = 16; o > 0; o >>= 1) {
        auto n = l;
        for (int k = 0; k < 32; ++k) {
            const VI other = (k + o < 32) ? l[k + o] : l[k];
            if (ref_better(other.v, other.i, l[k].v, l[k].i)) n[k] = other;
        }
        l = n;
    }
    return l[0];
}
inline VI model_block_argmax(const ArgCase& c, int B, int n) {
    const VI pad{-kInf, std::numeric_limits<std::int32_t>::max()};
    std::vector<VI> w(32, pad);
    for (int k = 0; k < B / 32; ++k) {
        std::vector<VI> l(32, pad);
        for (int t = 0; t < 32; ++t) if (32 * k + t < n) l[t] = {c.v[32 * k + t], c.id[32 * k + t]};
        w[k] = model_warp_argmax(l);
    }
    return model_warp_argmax(w);
}

// ---------------------------------------------------------------- Welford reference
struct MomRef { int n = 0; double mean = 0, m2 = 0, maxabs = 0, kappa = 0; bool nan = false, inf = false; };
inline MomRef ref_moments(const std::vector<float>& x) { // mean = sum/n, m2 = sum (x-mean)^2 (exact sums)
    MomRef r;
    std::vector<double> d;
    for (float f : x) { r.nan |= std::isnan(f); r.inf |= std::isinf(f); }
    r.n = int(x.size());
    if (r.nan || r.inf || x.empty()) return r;
    for (float f : x) { d.push_back(f); r.maxabs = std::max(r.maxabs, std::fabs(double(f))); }
    r.mean = fsum(d) / r.n;
    std::vector<double> dev, sq;
    for (double v : d) { dev.push_back((v - r.mean) * (v - r.mean)); sq.push_back(v * v); }
    r.m2    = fsum(dev);
    r.kappa = r.m2 > 0 ? std::sqrt(fsum(sq) / r.m2) : 0;
    return r;
}
// Bounds (u = 2^-24): mean error <= 8 n u max|x|; m2 error <= 8 n u kappa m2 with kappa =
// ||x||_2/sqrt(m2) (Chan-Golub-LeVeque, updating algorithms); m2 == 0 (constant data) must be exact.
inline bool moments_ok(const MomRef& r, float mean, float m2, int count, std::string& why) {
    const double u = std::ldexp(1.0, -24);
    if (count != r.n) { why = "count " + std::to_string(count) + " != " + std::to_string(r.n); return false; }
    if (r.n == 0) { if (mean != 0 || m2 != 0) { why = "empty must stay {0,0,0}"; return false; } return true; }
    if (r.nan) { if (!(std::isnan(mean) && std::isnan(m2))) { why = "NaN input must give NaN mean and m2"; return false; } return true; }
    if (r.inf) { if (std::isfinite(mean) && std::isfinite(m2)) { why = "Inf input must not give finite mean and m2"; return false; } return true; }
    const double mt = 8 * r.n * u * r.maxabs, vt = r.m2 == 0 ? 0 : 8 * r.n * u * (r.kappa + 1) * r.m2;
    if (!(std::fabs(double(mean) - r.mean) <= mt)) { why = "mean " + std::to_string(mean) + " vs " + std::to_string(r.mean) + " tol " + std::to_string(mt); return false; }
    if (!(std::fabs(double(m2) - r.m2) <= vt)) { why = "m2 " + std::to_string(m2) + " vs " + std::to_string(r.m2) + " tol " + std::to_string(vt); return false; }
    return true;
}

struct MomCase { std::string name; std::vector<float> x; };
inline std::vector<MomCase> make_moment_cases() {
    std::vector<MomCase> out;
    for (int n : {1, 2, 5, 31, 32, 33, 64, 100, 1000}) {
        auto add = [&](const char* kind, auto gen) {
            Rng rng(0x77ull * unsigned(n) + name_hash(kind));
            MomCase c{std::string(kind) + "/n" + std::to_string(n), std::vector<float>(n)};
            for (int i = 0; i < n; ++i) c.x[i] = gen(rng, i);
            out.push_back(std::move(c));
        };
        add("normal", [](Rng& r, int) { return (r.unit() + r.unit() + r.unit() - 1.5f) * 4.0f; });
        add("offset", [](Rng& r, int) { return 10000.0f + (r.unit() - 0.5f); });
        add("ints", [](Rng& r, int) { return float(r.uniform(-100, 100)); });
        add("constant", [](Rng&, int) { return -7.25f; });
        add("nan", [n](Rng& r, int i) { return i == n / 2 ? kNaN : r.unit(); });
        add("inf", [n](Rng& r, int i) { return i == n - 1 ? kInf : r.unit(); });
    }
    return out;
}
// per-lane element counts for warp_reduce_moments; every mode sums to n
inline std::vector<int> split_counts(int n, int mode, std::uint64_t seed) {
    std::vector<int> c(32, 0);
    Rng rng(seed);
    if (mode == 0) for (int l = 0; l < 32; ++l) c[l] = n / 32 + (l < n % 32);
    else if (mode == 1) c[0] = n;
    else if (mode == 2) { c[31] = n / 2; c[16] = n - n / 2; }
    else for (int i = 0; i < n; ++i) ++c[rng.uniform(0, 31)];
    return c;
}

} // namespace ninfer::test::common_ref
