#include "ninfer/ops/kda_input_proj.h"

#include <stdexcept>

namespace ninfer::ops {

void kda_input_proj_conv_update(const Tensor& x, const SingleProjectionWeight& qkv,
                                const Tensor& conv_weight, KdaConvUpdateState state,
                                const Tensor& valid_columns, KdaConvOutputs outputs,
                                MemoryArena& workspace, CudaExecutionView execution) {
    throw std::invalid_argument("kda_input_proj_conv_update: not supported on r9700");
}

void kda_input_proj_conv_record(const Tensor& x, const SingleProjectionWeight& qkv,
                                const Tensor& conv_weight, KdaConvSourceState state,
                                const Tensor& valid_columns, KdaConvOutputs outputs,
                                Tensor& conv_record, MemoryArena& workspace,
                                CudaExecutionView execution) {
    throw std::invalid_argument("kda_input_proj_conv_record: not supported on r9700");
}

std::size_t kda_input_proj_conv_update_workspace_capacity_bytes(const LinearProfile& qkv, int batch,
                                                                int min_tokens, int max_tokens) {
    throw std::invalid_argument(
        "kda_input_proj_conv_update_workspace_capacity_bytes: not supported on r9700");
}

std::size_t kda_input_proj_conv_record_workspace_capacity_bytes(const LinearProfile& qkv, int batch,
                                                                int min_tokens, int max_tokens) {
    throw std::invalid_argument(
        "kda_input_proj_conv_record_workspace_capacity_bytes: not supported on r9700");
}

} // namespace ninfer::ops
