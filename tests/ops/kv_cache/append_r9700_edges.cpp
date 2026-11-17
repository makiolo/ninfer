// R97-14 oracle cases for the D256 paged KV cache (BF16 and FP8 G128/UE8M0), Hkv 4|2.
// Exact byte references: E4M3FN RNE SATFINITE (float_codec.h), UE8M0 scale
// (fp8_cache_reference.h) and an order-explicit FP32 normalized Sylvester H256 for K. All three
// were crosschecked against exact rational references. Every case runs both kv_cache_append and
// kv_cache_append_prefix (one live batch row plus one zero-count row with invalid selectors) and
// compares every byte of every plane, including sentinel-filled untouched rows and pages.
#include "ops/suite.h"
#include "../../../tools/support/paged_kv_fixture.h"
#include "kv_cache_storage.h"
#include "ops/float_codec.h"
#include "ops/fp8_cache_reference.h"
#include "ninfer/ops/kv_cache_append.h"
#include "cuda_fixture.h"
#include "core/buffer.h"
#include "ops/op_tester.h"
#include "core/device.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <vector>

using namespace ninfer;
using namespace ninfer::test;
using namespace ninfer::test::float_codec;

namespace {

constexpr int kPage = 64;
constexpr int kDim  = 256;

struct PlaneShape {
    DType dtype;
    int extent;
};

// Planes: K codes, V codes, K scales, V scales (scales absent for BF16 storage).
std::array<PlaneShape, 4> planes_of(KvCacheStorage storage) {
    if (storage == KvCacheStorage::BFloat16)
        return {{{DType::BF16, kDim}, {DType::FP16, kDim}, {DType::U8, 0}, {DType::U8, 0}}};
    return {{{DType::FP8_E4M3FN, kDim}, {DType::FP8_E4M3FN, kDim}, {DType::U8, 2}, {DType::U8, 2}}};
}

std::size_t cache_index(int extent, int leading, int head, int position, int page, int heads) {
    return std::size_t(leading) +
           std::size_t(extent) *
               (std::size_t(position % kPage) + std::size_t(kPage) * (head + std::size_t(heads) * page));
}

// Logical bit 0 first through bit 7, then the exact 2^-4 normalization (contract FP32 order).
void normalized_hadamard_d256(std::array<float, kDim>& x) {
    for (int s = 1; s < kDim; s <<= 1)
        for (int i = 0; i < kDim; ++i)
            if ((i & s) == 0) {
                const float lo = x[i], hi = x[i + s];
                x[i]     = lo + hi;
                x[i + s] = lo - hi;
            }
    for (float& v : x) v *= 0x1p-4f;
}

void encode_fp8_row(std::array<float, kDim> x, bool rotate, std::vector<std::uint8_t>& codes,
                    std::vector<std::uint8_t>& scales, int head, int position, int page, int heads) {
    if (rotate) normalized_hadamard_d256(x);
    for (int group = 0; group < 2; ++group) {
        float absmax = 0;
        for (int d = 128 * group; d < 128 * (group + 1); ++d) absmax = std::max(absmax, std::abs(x[d]));
        const auto scale        = fp8_cache_scale(absmax);
        const double multiplier = fp8_cache_multiplier(scale);
        for (int d = 128 * group; d < 128 * (group + 1); ++d)
            codes[cache_index(kDim, d, head, position, page, heads)] =
                encode_e4m3fn_rne_satfinite(float(x[d] / multiplier));
        scales[cache_index(2, group, head, position, page, heads)] = scale;
    }
}

bool f16_nan(std::uint16_t h) { return (h & 0x7c00u) == 0x7c00u && (h & 0x03ffu) != 0; }

// One KV input set: BF16 bits [256, heads, tokens] for K and V.
struct Input {
    int heads;
    int tokens;
    std::vector<std::uint16_t> k, v;
};

class Fixture {
public:
    Fixture(KvCacheStorage storage, int heads, int pages, std::span<const std::int32_t> mapping,
            std::array<GuardedDeviceBuffer, 4>& dense)
        : shapes_(planes_of(storage)), packed_(dense[0].span().device, geometry(heads), pages),
          offsets_host_(packed_.table_offsets(mapping)), offsets_(to_device(offsets_host_)),
          table_(offsets_.span(), DType::I64, {std::int32_t(mapping.size()), 1}) {
        for (int i = 0; i < 4; ++i)
            if (shapes_[i].extent)
                dense_[i] = Tensor(dense[i].span(), shapes_[i].dtype, {shapes_[i].extent, kPage, heads, pages});
    }

    PagedPlaneView plane(int i) { return dense_[i].data ? packed_.plane(i, table_) : PagedPlaneView{}; }
    void upload() {
        for (int i = 0; i < 4; ++i)
            if (dense_[i].data) packed_.upload_plane(i, dense_[i]);
    }
    void copy_back() {
        for (int i = 0; i < 4; ++i)
            if (dense_[i].data) packed_.download_plane(i, dense_[i]);
    }
    bool guards_intact() const { return packed_.guards_intact(); }
    int verify_table() const {
        return verify_exact("page table unchanged", from_device<std::int64_t>(offsets_, offsets_host_.size()),
                            offsets_host_);
    }

private:
    KVPageGeometry geometry(int heads) const {
        KVPageGeometry result;
        for (const auto& shape : shapes_)
            if (shape.extent) result.planes.push_back({shape.dtype, shape.extent, heads});
        return result;
    }

    std::array<PlaneShape, 4> shapes_;
    tool::PackedKVFixture packed_;
    std::vector<std::int64_t> offsets_host_;
    DeviceBuffer offsets_;
    Tensor table_;
    std::array<Tensor, 4> dense_;
};

// first_position selects partial first/last pages; logical page p maps to physical 2p+1 so even
// pages and every unaddressed row of addressed pages keep their sentinel bytes.
int run_case(const std::string& label, KvCacheStorage storage, const Input& in, int first_position,
             bool prefix) {
    const int heads = in.heads, tokens = in.tokens;
    const int logical_pages  = (first_position + tokens + kPage - 1) / kPage;
    const int physical_pages = 2 * logical_pages + 1;
    std::vector<std::int32_t> mapping(logical_pages);
    for (int p = 0; p < logical_pages; ++p) mapping[p] = 2 * p + 1;

    const auto shapes = planes_of(storage);
    std::array<std::size_t, 4> bytes{};
    std::array<std::vector<std::uint8_t>, 4> expected;
    for (int i = 0; i < 4; ++i) {
        bytes[i] = std::size_t(shapes[i].extent) * kPage * heads * physical_pages * dtype_size(shapes[i].dtype);
        expected[i].assign(bytes[i], std::uint8_t(0xa5 + 17 * i));
    }
    std::array<GuardedDeviceBuffer, 4> dense{
        GuardedDeviceBuffer(bytes[0]), GuardedDeviceBuffer(bytes[1]),
        GuardedDeviceBuffer(std::max<std::size_t>(1, bytes[2])),
        GuardedDeviceBuffer(std::max<std::size_t>(1, bytes[3]))};
    for (int i = 0; i < 4; ++i)
        if (bytes[i]) dense[i].copy_from_host(expected[i].data(), bytes[i]);
    Fixture fixture(storage, heads, physical_pages, mapping, dense);
    fixture.upload();

    // Expected representation of every live row.
    std::vector<std::uint16_t> k_bf16, v_f16;
    if (storage == KvCacheStorage::BFloat16) {
        k_bf16.resize(bytes[0] / 2);
        v_f16.resize(bytes[1] / 2);
        std::memcpy(k_bf16.data(), expected[0].data(), bytes[0]);
        std::memcpy(v_f16.data(), expected[1].data(), bytes[1]);
    }
    for (int t = 0; t < tokens; ++t) {
        const int position = first_position + t;
        const int page     = mapping[position / kPage];
        for (int h = 0; h < heads; ++h) {
            const std::size_t base = std::size_t(kDim) * (h + std::size_t(heads) * t);
            if (storage == KvCacheStorage::BFloat16) {
                for (int d = 0; d < kDim; ++d) {
                    const auto target = cache_index(kDim, d, h, position, page, heads);
                    k_bf16[target]    = in.k[base + d];
                    v_f16[target]     = f32_to_f16_bits(bf16_to_f32(in.v[base + d]));
                }
                continue;
            }
            std::array<float, kDim> kr{}, vr{};
            for (int d = 0; d < kDim; ++d) {
                kr[d] = bf16_to_f32(in.k[base + d]);
                vr[d] = bf16_to_f32(in.v[base + d]);
            }
            encode_fp8_row(kr, true, expected[0], expected[2], h, position, page, heads);
            encode_fp8_row(vr, false, expected[1], expected[3], h, position, page, heads);
        }
    }
    if (storage == KvCacheStorage::BFloat16) {
        std::memcpy(expected[0].data(), k_bf16.data(), bytes[0]);
        std::memcpy(expected[1].data(), v_f16.data(), bytes[1]);
    }

    // Batch row 1 of the prefix launch: NaN K/V, count 0, position -1 and table row -1 are never read.
    const int batch = prefix ? 2 : 1;
    std::vector<std::uint16_t> k_in = in.k, v_in = in.v;
    k_in.resize(in.k.size() * batch, 0x7fc1u);
    v_in.resize(in.v.size() * batch, 0xffc3u);
    std::vector<std::int32_t> positions(std::size_t(tokens) * batch, -1);
    for (int t = 0; t < tokens; ++t) positions[t] = first_position + t;
    const std::vector<std::int32_t> counts{tokens, 0}, rows{0, -1};
    DeviceBuffer d_k = to_device(k_in), d_v = to_device(v_in), d_positions = to_device(positions);
    DeviceBuffer d_counts = to_device(counts), d_rows = to_device(rows);
    Tensor k(d_k.span(), DType::BF16, {kDim, heads, tokens, batch});
    Tensor v(d_v.span(), DType::BF16, {kDim, heads, tokens, batch});
    if (prefix) {
        const PagedKVBatchLayerView cache{fixture.plane(0), fixture.plane(1), fixture.plane(2),
                                          fixture.plane(3), kDim, heads, storage};
        ops::kv_cache_append_prefix(k, v, Tensor(d_positions.span(), DType::I32, {tokens, batch}),
                                    Tensor(d_counts.span(), DType::I32, {batch}),
                                    Tensor(d_rows.span(), DType::I32, {batch}),
                                    {0, static_cast<std::uint32_t>(tokens)}, cache,
                                    cuda_environment().default_execution);
    } else {
        const PagedKVLayerView cache{fixture.plane(0), fixture.plane(1), fixture.plane(2),
                                     fixture.plane(3), kDim, heads, storage};
        ops::kv_cache_append(k.view({kDim, heads, tokens}), v.view({kDim, heads, tokens}),
                             Tensor(d_positions.span(), DType::I32, {tokens}), cache,
                             cuda_environment().default_execution);
    }
    cuda_synchronize();
    fixture.copy_back();

    int failures = 0;
    const std::string id = label + (prefix ? " prefix" : " append") + " Hkv=" + std::to_string(heads) +
                           " T=" + std::to_string(tokens) + " P=" + std::to_string(first_position);
    const std::array<const char*, 4> names{" k codes", " v codes", " k scales", " v scales"};
    for (int i = 0; i < 4; ++i) {
        if (!bytes[i]) continue;
        auto got = from_device<std::uint8_t>(dense[i].data(), bytes[i]);
        if (storage == KvCacheStorage::BFloat16 && i == 1) {
            // FP16_RNE(NaN) is any FP16 NaN; every other V word is exact.
            for (std::size_t w = 0; w < bytes[1] / 2; ++w) {
                std::uint16_t g, e;
                std::memcpy(&g, &got[2 * w], 2);
                std::memcpy(&e, &expected[1][2 * w], 2);
                if (f16_nan(e) && f16_nan(g)) std::memcpy(&got[2 * w], &e, 2);
            }
        }
        failures += verify_exact((id + names[i]).c_str(), got, expected[i]);
        failures += dense[i].verify_guards((id + names[i] + " guards").c_str());
    }
    failures += !fixture.guards_intact();
    failures += fixture.verify_table();
    failures += verify_exact((id + " k input unchanged").c_str(), from_device<std::uint16_t>(d_k, k_in.size()), k_in);
    failures += verify_exact((id + " v input unchanged").c_str(), from_device<std::uint16_t>(d_v, v_in.size()), v_in);
    return failures;
}

Input make_input(int heads, int tokens) {
    const std::size_t n = std::size_t(kDim) * heads * tokens;
    return {heads, tokens, std::vector<std::uint16_t>(n), std::vector<std::uint16_t>(n)};
}

// Every positive E4M3FN code value, every midpoint between neighbours and both BF16 neighbours of
// each, signed, in 127-value G128 groups whose element 0 pins absmax at 448*2^e (or one BF16 ulp
// either side, moving the scale seam). Groups use scales from 2^-110 to 2^100. K receives the
// same rows rotated, so the K codes cover the Hadamard path with a wide dynamic range.
Input fp8_codec_boundaries(int heads) {
    std::vector<float> unit;
    for (int c = 0; c < 127; ++c) {
        const float value = decode_e4m3fn(std::uint8_t(c));
        unit.push_back(value);
        if (c == 126) break;
        const float mid  = 0.5f * (value + decode_e4m3fn(std::uint8_t(c + 1)));
        const auto bits  = f32_to_bf16(mid);
        unit.insert(unit.end(), {mid, bf16_to_f32(std::uint16_t(bits + 1)), bf16_to_f32(std::uint16_t(bits - 1))});
    }
    std::vector<float> values;
    for (float u : unit) values.insert(values.end(), {u, -u});
    const std::array<float, 4> pins{448.0f, -448.0f, 450.0f, 446.0f};
    const std::array<int, 8> exponents{0, -9, 17, -40, 64, -100, 100, -110};
    const int groups = int((values.size() + 126) / 127);
    const int tokens = (groups + 2 * heads - 1) / (2 * heads);
    Input in         = make_input(heads, tokens);
    for (int g = 0; g < 2 * heads * tokens; ++g) {
        const float factor = std::ldexp(1.0f, exponents[g % exponents.size()]);
        for (int i = 0; i < 128; ++i) {
            const std::size_t source = std::size_t(g) * 127 + i - 1;
            const float u = i == 0 ? pins[g % pins.size()] : source < values.size() ? values[source] : 0.0f;
            in.v[std::size_t(g) * 128 + i] = f32_to_bf16(u * factor);
        }
    }
    in.k = in.v;
    return in;
}

// Rows of BF16 subnormals (absmax subnormal -> scale byte 0), tiny normals (scale clamp at 2^-127),
// signed zeros (scale byte 127), BF16 max next to subnormals and min normals (scale 247). These
// fail on a device that flushes FP32 denormals in the Hadamard adds or the scale multiply.
Input fp8_tiny_and_huge(int heads) {
    Input in = make_input(heads, 2);
    for (int g = 0; g < 4 * heads; ++g)
        for (int i = 0; i < 128; ++i) {
            std::uint16_t bits = 0;
            switch (g % 4) {
            case 0: bits = std::uint16_t((i % 127 + 1) | (i & 1 ? 0x8000u : 0)); break;
            case 1: bits = std::uint16_t(i == 0 ? 0x0380u : (0x0080u + i) | (i & 2 ? 0x8000u : 0)); break;
            case 2: bits = i & 1 ? 0x8000u : 0x0000u; break;
            case 3: bits = i == 0 ? 0x7f7fu : i < 64 ? 0x0080u + i : (0x0001u + i) | 0x8000u; break;
            }
            in.v[std::size_t(g) * 128 + i] = bits;
        }
    in.k = in.v;
    return in;
}

// Random BF16 with exponents spread over 2^-24..2^24: H256 sums round in FP32, so any butterfly
// reordering, FMA contraction or lane permutation changes K bytes.
Input wide_random(int heads, int tokens, std::uint32_t seed) {
    Input in = make_input(heads, tokens);
    std::vector<float> a(in.k.size()), b(in.k.size());
    fill_uniform(a, seed, -2.0f, 2.0f);
    fill_uniform(b, seed ^ 0x9e3779b9u, -24.0f, 24.0f);
    for (std::size_t i = 0; i < a.size(); ++i) {
        in.k[i] = f32_to_bf16(a[i] * std::exp2(std::round(b[i])));
        in.v[i] = f32_to_bf16(a[(i * 7919) % a.size()] * std::exp2(std::round(b[i]) * 0.5f));
    }
    return in;
}

// BF16 storage: all 65536 BF16 bit patterns as K (bit copy, including NaN payloads) and V
// (FP16 RNE: overflow to inf, FP16 subnormals, ties, signed zeros, NaN class).
Input bf16_all_patterns(int heads) {
    const int tokens = 65536 / (kDim * heads);
    Input in         = make_input(heads, tokens);
    for (std::size_t i = 0; i < in.k.size(); ++i) {
        in.k[i] = std::uint16_t(i);
        in.v[i] = std::uint16_t((i * 40503u) & 0xffffu); // odd multiplier: a permutation
    }
    return in;
}

void add_cases(OpSuite& suite) {
    using S        = KvCacheStorage;
    const auto add = [&](std::string id, S storage, auto input, int first_position) {
        const std::string format = kv_cache_storage_name(storage);
        for (bool prefix : {false, true})
            suite.add("append/r9700/" + format + "/" + id + (prefix ? "/prefix" : "/append"),
                      [=] { return run_case(id, storage, input(), first_position, prefix); },
                      {{"format", format}});
    };
    for (int heads : {4, 2}) {
        const auto h = "h" + std::to_string(heads);
        add("codec-boundaries/" + h, S::Fp8E4m3fnGroup128Ue8m0, [=] { return fp8_codec_boundaries(heads); }, 59);
        add("tiny-huge/" + h, S::Fp8E4m3fnGroup128Ue8m0, [=] { return fp8_tiny_and_huge(heads); }, 63);
        add("wide-random-t129/" + h, S::Fp8E4m3fnGroup128Ue8m0, [=] { return wide_random(heads, 129, 0x1497u); }, 61);
        add("all-patterns/" + h, S::BFloat16, [=] { return bf16_all_patterns(heads); }, 37);
    }
}

} // namespace

int main(int argc, char** argv) {
    OpSuite suite;
    add_cases(suite);
    return suite.main(argc, argv);
}
