#include "ninfer/ops/moe.h"

#include <stdexcept>

namespace ninfer::ops {

std::size_t swiglu_workspace_capacity_bytes(const SwiGLUProfile& profile, std::int32_t min_tokens,
                                            std::int32_t max_tokens) {
    throw std::invalid_argument("swiglu_workspace_capacity_bytes: not supported on r9700");
}

void swiglu(const Tensor& x, const SwiGLUProfile& profile, const SwiGLUWeights& weights,
            Tensor& destination, MemoryArena& workspace, CudaExecutionView execution) {
    throw std::invalid_argument("swiglu: not supported on r9700");
}

std::size_t topk_sigmoid_swiglu_moe_workspace_capacity_bytes(const SigmoidMoeProfile&,
                                                             ExpertSubsetKind,
                                                             std::int32_t min_tokens,
                                                             std::int32_t max_tokens) {
    throw std::invalid_argument(
        "topk_sigmoid_swiglu_moe_workspace_capacity_bytes: not supported on r9700");
}

void topk_sigmoid_swiglu_moe(const Tensor&, const SigmoidMoeProfile&, const SigmoidMoeWeights&,
                             ExpertSubset, Tensor& destination, MoeTokenValidity, MemoryArena&,
                             CudaExecutionView) {
    throw std::invalid_argument("topk_sigmoid_swiglu_moe: not supported on r9700");
}

std::size_t topk_softmax_swiglu_moe_workspace_capacity_bytes(const MoeProfile& profile,
                                                             ExpertSubsetKind subset,
                                                             MoeEpilogue epilogue,
                                                             std::int32_t min_tokens,
                                                             std::int32_t max_tokens) {
    throw std::invalid_argument(
        "topk_softmax_swiglu_moe_workspace_capacity_bytes: not supported on r9700");
}

void topk_softmax_swiglu_moe(const Tensor& x, const MoeProfile& profile, const MoeWeights& weights,
                             ExpertSubset subset, MoeEpilogue epilogue, Tensor& destination,
                             MoeTokenValidity validity, MemoryArena& workspace,
                             CudaExecutionView execution, const MoeExecutionHints& hints) {
    throw std::invalid_argument("topk_softmax_swiglu_moe: not supported on r9700");
}

} // namespace ninfer::ops
