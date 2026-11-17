#include "ninfer/ops/softmax_attention.h"

#include <stdexcept>

namespace ninfer::ops {

// These dense families have no r9700 implementation yet. Keep their symbols
// available without compiling their CUDA/PTX launchers or advertising support.
void softmax_attention(const Tensor&, const Tensor&, const Tensor&, AttentionHeadGeometry,
                       float, MemoryArena&, Tensor&, CudaExecutionView) {
    throw std::invalid_argument("softmax_attention: plain attention is not supported on r9700");
}

void packed_softmax_attention(const Tensor&, const Tensor&, const Tensor&, AttentionHeadGeometry,
                              float, const Tensor&, MemoryArena&, Tensor&, CudaExecutionView) {
    throw std::invalid_argument("packed_softmax_attention: not supported on r9700");
}

void packed_softmax_attention(const Tensor&, const Tensor&, const Tensor&, AttentionHeadGeometry,
                              float, std::int32_t, Tensor&, CudaExecutionView) {
    throw std::invalid_argument("packed_softmax_attention: not supported on r9700");
}

std::size_t packed_softmax_attention_workspace_capacity_bytes(AttentionHeadGeometry,
                                                              std::int32_t, std::int32_t,
                                                              std::int32_t, std::int32_t) {
    throw std::invalid_argument("packed_softmax_attention: not supported on r9700");
}

void context_softmax_attention(const Tensor&, const Tensor&, const Tensor&, const Tensor&,
                               const Tensor&, const Tensor&, AttentionHeadGeometry, float,
                               const ConstPagedKVBatchLayerView&, ContextAttentionExecutionEnvelope,
                               MemoryArena&, Tensor&, CudaExecutionView) {
    throw std::invalid_argument("context_softmax_attention: not supported on r9700");
}

std::size_t context_softmax_attention_workspace_capacity_bytes(AttentionHeadGeometry,
    ContextAttentionExecutionEnvelope, std::int32_t, std::int32_t, std::int32_t) {
    throw std::invalid_argument("context_softmax_attention: not supported on r9700");
}

} // namespace ninfer::ops
