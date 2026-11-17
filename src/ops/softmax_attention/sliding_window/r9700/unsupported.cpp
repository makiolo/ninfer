#include "ninfer/ops/sliding_window_attention.h"

#include <stdexcept>

namespace ninfer::ops {

void sliding_window_attention(const Tensor& q, const Tensor& query_k, const Tensor& query_v,
                              const Tensor& positions, const Tensor& valid_columns,
                              AttentionHeadGeometry geometry, std::uint32_t window, float scale,
                              const ConstCyclicKVCacheLayerView& context,
                              SlidingWindowAttentionExecutionEnvelope envelope,
                              MemoryArena& workspace, Tensor& out, CudaExecutionView execution) {
    throw std::invalid_argument("sliding_window_attention: not supported on r9700");
}

std::size_t sliding_window_attention_workspace_capacity_bytes(
    AttentionHeadGeometry geometry, std::uint32_t window,
    SlidingWindowAttentionExecutionEnvelope envelope, std::int32_t min_tokens,
    std::int32_t max_tokens, std::int32_t batch_size) {
    throw std::invalid_argument(
        "sliding_window_attention_workspace_capacity_bytes: not supported on r9700");
}

} // namespace ninfer::ops
