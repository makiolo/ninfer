#include "ops/suite.h"
#include "../../../tools/support/paged_kv_fixture.h"
#include "core/paged_kv_storage.h"
#include "ninfer/ops/kv_cache_append.h"
#include "cuda_fixture.h"
#include "ops/op_tester.h"
#include "kv_cache_storage.h"

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

using namespace ninfer;
using namespace ninfer::test;

namespace {
// Valid public D256 operands reach the storage dispatcher. Compare the entire
// record backing, including gaps, after rejection of both public append forms.
int reject(KvCacheStorage storage, int heads, bool prefix, int tokens) {
    const auto layout = paged_kv_storage_layout(storage, 256);
    KVPageGeometry geometry;
    geometry.planes = {{layout.key.data_dtype, layout.key.data_leading_extent, heads},
                       {layout.value.data_dtype, layout.value.data_leading_extent, heads},
                       {layout.key.scale_dtype, layout.key.scale_leading_extent, heads},
                       {layout.value.scale_dtype, layout.value.scale_leading_extent, heads}};
    const int pages = (tokens + 63) / 64;
    tool::PackedKVFixture fixture(cuda_environment().default_execution.device(), geometry, pages);
    std::vector<std::int32_t> mapping(pages);
    for (int i = 0; i < pages; ++i) mapping[i] = i;
    const auto offsets = fixture.table_offsets(mapping);
    auto d_offsets     = to_device(offsets);
    Tensor table(d_offsets.span(), DType::I64, {pages, 1});
    PagedKVLayerView cache{fixture.plane(0, table),
                           fixture.plane(1, table),
                           fixture.plane(2, table),
                           fixture.plane(3, table),
                           256,
                           heads,
                           storage};
    std::vector<std::uint16_t> bits(std::size_t(256) * heads * tokens, 0x3f80);
    std::vector<std::int32_t> positions(tokens);
    for (int i = 0; i < tokens; ++i) positions[i] = i;
    auto d_k = to_device(bits), d_v = to_device(bits), d_positions = to_device(positions);
    auto d_count = to_device(std::vector<std::int32_t>{tokens});
    auto d_row   = to_device(std::vector<std::int32_t>{0});
    Tensor k(d_k.span(), DType::BF16, {256, heads, tokens, 1});
    Tensor v(d_v.span(), DType::BF16, {256, heads, tokens, 1});
    Tensor pos(d_positions.span(), DType::I32, {tokens, 1});
    const auto before = from_device<std::uint8_t>(fixture.storage().data, fixture.storage().bytes);
    bool rejected     = false;
    try {
        if (prefix)
            ops::kv_cache_append_prefix(
                k, v, pos, Tensor(d_count.span(), DType::I32, {1}),
                Tensor(d_row.span(), DType::I32, {1}), {1, std::uint32_t(tokens)},
                single_row_paged_kv_batch_view(cache), cuda_environment().default_execution);
        else
            ops::kv_cache_append(k, v, pos, cache, cuda_environment().default_execution);
    } catch (const std::invalid_argument& e) {
        // Distinguish the unsupported storage decision from malformed fixture validation.
        rejected =
            std::string(e.what()).find("r9700 supports only BF16 and FP8") != std::string::npos;
    }
    CUDA_CHECK(cudaDeviceSynchronize());
    int failures = !rejected;
    failures += verify_exact(
        "rejected KV backing unchanged",
        from_device<std::uint8_t>(fixture.storage().data, fixture.storage().bytes), before);
    failures +=
        verify_exact("rejected K unchanged", from_device<std::uint16_t>(d_k, bits.size()), bits);
    failures +=
        verify_exact("rejected V unchanged", from_device<std::uint16_t>(d_v, bits.size()), bits);
    failures += verify_exact("rejected table unchanged",
                             from_device<std::int64_t>(d_offsets, offsets.size()), offsets);
    return failures;
}
} // namespace

int main(int argc, char** argv) {
    OpSuite suite;
    const std::array formats{KvCacheStorage::Int8Group64, KvCacheStorage::Nvfp4Group16,
                             KvCacheStorage::Fp8KeyNvfp4Value};
    for (auto storage : formats)
        for (int heads : {2, 4})
            for (bool prefix : {false, true})
                for (int tokens : {1, 129})
                    suite.add("append/rejection/" + std::string(kv_cache_storage_name(storage)) +
                                  "/h" + std::to_string(heads) + (prefix ? "/prefix/" : "/full/") +
                                  std::to_string(tokens),
                              [=] { return reject(storage, heads, prefix, tokens); },
                              {{"format", kv_cache_storage_name(storage)}});
    return suite.main(argc, argv);
}
