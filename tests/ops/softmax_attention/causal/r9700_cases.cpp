// Reference cases for the r9700 (gfx1201, wave32) dense causal attention route used by the
// Qwen3.5 Dense text and MTP paths. They reuse the public runners and the unchanged FP64 oracle;
// only shapes, lengths and Q/K amplitudes are new. Scope: the 27B head profile [256,24,4] and
// the KV storages planned for r9700 (BF16 and FP8-E4M3FN-G128/UE8M0).
#include "fixture.h"
#include "kv_cache_storage.h"

namespace ninfer::test::causal_attention {
namespace {

constexpr const Geometry& kQwen27b = kGeometries[0]; // d256-h24-kv4, GQA group 6

std::string suffix(const AttentionCase& c, MappingPattern mapping) {
    return "/t" + std::to_string(c.tokens) + "/base" + std::to_string(c.base) + "/bound" +
           std::to_string(c.envelope_max) + "/seed" + std::to_string(c.seed) + "/amp" +
           std::to_string(static_cast<int>(c.qk_amplitude * 100.0f)) + "/" +
           mapping_name(mapping);
}

void add_append(OpSuite& suite, KvCacheStorage storage, AttentionCase c, MappingPattern mapping,
                std::vector<int> queries = {}) {
    suite.add(
        std::string("r9700_causal_append/") + cache_name(storage) + "/" + kQwen27b.name +
            suffix(c, mapping),
        [=] {
            return run_append_case(cuda_environment().context.execution_view(), kQwen27b,
                                   storage, c, mapping, queries);
        },
        {{"suite", "r9700"},
         {"format", kv_cache_storage_name(storage)},
         {"execution", c.graph_replay ? "graph" : "eager"}});
}

void add_cached(OpSuite& suite, KvCacheStorage storage, AttentionCase c, MappingPattern mapping,
                std::vector<int> queries = {}) {
    suite.add(
        std::string("r9700_causal_cached/") + cache_name(storage) + "/" + kQwen27b.name +
            suffix(c, mapping),
        [=] {
            return run_cached_case(cuda_environment().context.execution_view(), kQwen27b,
                                   storage, c, mapping, queries);
        },
        {{"suite", "r9700"},
         {"format", kv_cache_storage_name(storage)},
         {"execution", c.graph_replay ? "graph" : "eager"}});
}

void add_batch(OpSuite& suite, KvCacheStorage storage, BatchAttentionCase c, int envelope = 0) {
    std::string id = std::string("r9700_causal_batch/") + cache_name(storage) + "/" +
                     kQwen27b.name + "/w" + std::to_string(c.width) + "/seed" +
                     std::to_string(c.seed) + "/contexts";
    for (int context : c.contexts) id += "_" + std::to_string(context);
    suite.add(
        id,
        [=] {
            return run_batch_case(cuda_environment().context.execution_view(), kQwen27b, storage,
                                  c, envelope);
        },
        {{"suite", "r9700"},
         {"format", kv_cache_storage_name(storage)},
         {"execution", c.graph_replay ? "graph" : "eager"}});
}

// Visible lengths that straddle 32-lane, 64-lane and 64-row page boundaries (p+1 = base+1).
void add_unaligned_decode(OpSuite& suite, KvCacheStorage storage) {
    std::uint32_t seed = 13001u;
    for (int base : {30, 31, 32, 33, 62, 63, 64, 65, 94, 95, 96, 97, 158, 159, 160, 161, 4094, 4095,
                     4096, 4097})
        add_append(suite, storage, {1, base, static_cast<unsigned>(base + 1), seed++},
                   MappingPattern::Fragmented);
}

// Prefill and chunked prefill: the chunk starts at an unaligned history length, so the causal
// diagonal crosses lane, tile and page boundaries at different query rows.
void add_unaligned_prefill(OpSuite& suite, KvCacheStorage storage) {
    const AttentionCase append_cases[] = {
        {31, 0, 31, 13101u},    {33, 0, 33, 13102u},   {95, 0, 95, 13103u},
        {97, 33, 130, 13104u},  {33, 31, 64, 13105u},  {64, 33, 97, 13106u},
        {65, 95, 160, 13107u},  {161, 97, 258, 13108u}, {129, 4033, 4162, 13109u},
    };
    for (const auto& c : append_cases) add_append(suite, storage, c, MappingPattern::Fragmented);
    const AttentionCase cached_cases[] = {
        {33, 31, 64, 13151u},
        {97, 161, 258, 13152u},
        {31, 4097, 4128, 13153u},
    };
    for (const auto& c : cached_cases) add_cached(suite, storage, c, MappingPattern::Offset);
    // Both halves of a 161-token prompt split at an unaligned point (0..60, then 61..160); each
    // call runs against its own seeded history, so this checks the split shapes, not state carry.
    add_append(suite, storage, {61, 0, 161, 13110u}, MappingPattern::Fragmented);
    add_append(suite, storage, {100, 61, 161, 13111u}, MappingPattern::Fragmented);
    // Mixed-length MTP-style batch: draft widths 1..4 over unaligned histories.
    add_batch(suite, storage,
              {4,
               {33, 95, 161, 4097, 0},
               {4, 3, 1, 2, 0},
               {3, 0, 4, 1, 2},
               MappingPattern::Fragmented,
               13112u,
               true});
}

// Long context up to the 27B position limit (262144 visible keys). Only selected query columns
// enter the FP64 oracle; execution, cache and guard checks still cover every column.
void add_long_context(OpSuite& suite, KvCacheStorage storage) {
    add_cached(suite, storage, {1, 262143, 262144, 13201u}, MappingPattern::Fragmented);
    add_append(suite, storage, {1, 262143, 262144, 13202u}, MappingPattern::Fragmented);
    add_append(suite, storage, {1, 100002, 262144, 13203u}, MappingPattern::Fragmented);
    add_append(suite, storage, {67, 131005, 131072, 13204u}, MappingPattern::Fragmented,
               {0, 31, 32, 63, 64, 66});
}

// Large logits stress the running maximum: with Q/K uniform in [-a,a], D=256 and scale 1/16 the
// logit standard deviation is a^2/3. a=10 gives max logits ~150 and a=16 ~400, both beyond FP32
// exp overflow (88.7) without max subtraction. BF16 only: native FP8 Q quantization error scales
// with |logit|, which is outside the FP8 conformance budget.
void add_large_logits(OpSuite& suite) {
    const auto storage = KvCacheStorage::BFloat16;
    add_append(suite, storage, {1, 1000, 1001, 13301u, false, false, 10.0f},
               MappingPattern::Fragmented);
    add_append(suite, storage, {37, 61, 98, 13302u, false, false, 10.0f},
               MappingPattern::Fragmented);
    add_append(suite, storage, {1, 8191, 8192, 13303u, false, false, 16.0f},
               MappingPattern::Fragmented);
    add_cached(suite, storage, {7, 4093, 4100, 13304u, false, false, 10.0f},
               MappingPattern::Offset);
    add_append(suite, storage, {129, 33, 162, 13305u, false, false, 16.0f},
               MappingPattern::Fragmented, {0, 31, 64, 128});
}

} // namespace

void add_r9700_cases(OpSuite& suite) {
    for (auto storage : {KvCacheStorage::BFloat16, KvCacheStorage::Fp8E4m3fnGroup128Ue8m0}) {
        add_unaligned_decode(suite, storage);
        add_unaligned_prefill(suite, storage);
        add_long_context(suite, storage);
    }
    add_large_logits(suite);
}

} // namespace ninfer::test::causal_attention
