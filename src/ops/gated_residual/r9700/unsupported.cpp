#include "ninfer/ops/gated_residual.h"

#include <stdexcept>

namespace ninfer::ops {

void hc_expand_norm(const Tensor& embedding, const Tensor& norm_weight, float epsilon,
                    Tensor& residual, Tensor& normalized, CudaExecutionView execution) {
    throw std::invalid_argument("hc_expand_norm: not supported on r9700");
}

void hc_grouped_rmsnorm(const Tensor& residual, const Tensor& norm_weight, float epsilon,
                        Tensor& normalized, CudaExecutionView execution) {
    throw std::invalid_argument("hc_grouped_rmsnorm: not supported on r9700");
}

void hc_combine(const Tensor& residual, const Tensor& block_output, const Tensor& injection,
                Tensor& updated_residual, CudaExecutionView execution) {
    throw std::invalid_argument("hc_combine: not supported on r9700");
}

void hc_combine_norm(const Tensor& residual, const Tensor& block_output, const Tensor& injection,
                     const Tensor& next_norm_weight, float epsilon, Tensor& updated_residual,
                     Tensor& normalized, CudaExecutionView execution) {
    throw std::invalid_argument("hc_combine_norm: not supported on r9700");
}

void hc_combine_norm(const Tensor& residual, const Tensor& block_output,
                     const Tensor& next_norm_weight, float epsilon, Tensor& updated_residual,
                     Tensor& normalized, CudaExecutionView execution) {
    throw std::invalid_argument("hc_combine_norm: not supported on r9700");
}

void hc_mix(const Tensor& normalized, const HcBf16MixWeights& weights, MemoryArena& workspace,
            Tensor& block_input, Tensor& injection, CudaExecutionView execution) {
    throw std::invalid_argument("hc_mix: not supported on r9700");
}

void hc_mix(const Tensor& normalized, const HcBf16FinalMixWeights& weights, MemoryArena& workspace,
            Tensor& sample_hidden, CudaExecutionView execution) {
    throw std::invalid_argument("hc_mix: not supported on r9700");
}

void hc_mix(const Tensor& normalized, const HcFp8MixWeights& weights, MemoryArena& workspace,
            Tensor& block_input, Tensor& injection, CudaExecutionView execution) {
    throw std::invalid_argument("hc_mix: not supported on r9700");
}

void hc_mix(const Tensor& normalized, const HcFp8FinalMixWeights& weights, MemoryArena& workspace,
            Tensor& sample_hidden, CudaExecutionView execution) {
    throw std::invalid_argument("hc_mix: not supported on r9700");
}

std::size_t hc_mix_workspace_capacity_bytes(const HcFp8MixWeights& weights, std::int32_t min_tokens,
                                            std::int32_t max_tokens) {
    throw std::invalid_argument("hc_mix_workspace_capacity_bytes: not supported on r9700");
}

std::size_t hc_mix_workspace_capacity_bytes(const HcFp8FinalMixWeights& weights,
                                            std::int32_t min_tokens, std::int32_t max_tokens) {
    throw std::invalid_argument("hc_mix_workspace_capacity_bytes: not supported on r9700");
}

std::size_t hc_mix_workspace_capacity_bytes(const HcFp8MixProfile& profile, std::int32_t min_tokens,
                                            std::int32_t max_tokens) {
    throw std::invalid_argument("hc_mix_workspace_capacity_bytes: not supported on r9700");
}

std::size_t hc_mix_workspace_capacity_bytes(const HcFp8FinalMixProfile& profile,
                                            std::int32_t min_tokens, std::int32_t max_tokens) {
    throw std::invalid_argument("hc_mix_workspace_capacity_bytes: not supported on r9700");
}

std::size_t hc_mix_workspace_capacity_bytes(const HcBf16MixWeights& weights,
                                            std::int32_t min_tokens, std::int32_t max_tokens) {
    throw std::invalid_argument("hc_mix_workspace_capacity_bytes: not supported on r9700");
}

std::size_t hc_mix_workspace_capacity_bytes(const HcBf16FinalMixWeights& weights,
                                            std::int32_t min_tokens, std::int32_t max_tokens) {
    throw std::invalid_argument("hc_mix_workspace_capacity_bytes: not supported on r9700");
}

std::size_t hc_mix_workspace_capacity_bytes(const HcBf16MixProfile& profile,
                                            std::int32_t min_tokens, std::int32_t max_tokens) {
    throw std::invalid_argument("hc_mix_workspace_capacity_bytes: not supported on r9700");
}

std::size_t hc_mix_workspace_capacity_bytes(const HcBf16FinalMixProfile& profile,
                                            std::int32_t min_tokens, std::int32_t max_tokens) {
    throw std::invalid_argument("hc_mix_workspace_capacity_bytes: not supported on r9700");
}

} // namespace ninfer::ops
