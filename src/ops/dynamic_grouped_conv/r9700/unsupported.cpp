#include "ninfer/ops/dynamic_grouped_conv.h"

#include <stdexcept>

namespace ninfer::ops {

std::size_t rmsnorm_dynamic_grouped_conv_prepare_workspace_capacity_bytes(
    std::int32_t hidden, std::int32_t min_width, std::int32_t max_width,
    std::int32_t min_batch_size, std::int32_t max_batch_size) {
    throw std::invalid_argument(
        "rmsnorm_dynamic_grouped_conv_prepare_workspace_capacity_bytes: not supported on r9700");
}

void rmsnorm_dynamic_grouped_conv_prepare(const Tensor& residual, const Tensor& norm_weight,
                                          float eps, const Tensor& base_kernel,
                                          const Weight& kernel_projection_weight, Tensor& prepared,
                                          Tensor& finish_delta, MemoryArena& workspace,
                                          CudaExecutionView execution) {
    throw std::invalid_argument("rmsnorm_dynamic_grouped_conv_prepare: not supported on r9700");
}

std::size_t linear_dynamic_grouped_conv_add_workspace_capacity_bytes(std::int32_t input_rows,
                                                                     std::int32_t min_width,
                                                                     std::int32_t max_width,
                                                                     std::int32_t min_batch_size,
                                                                     std::int32_t max_batch_size) {
    throw std::invalid_argument(
        "linear_dynamic_grouped_conv_add_workspace_capacity_bytes: not supported on r9700");
}

void linear_dynamic_grouped_conv_add(const Tensor& x, const Weight& projection_weight,
                                     const Tensor& base_kernel, const Tensor& finish_delta,
                                     Tensor& residual, MemoryArena& workspace,
                                     CudaExecutionView execution) {
    throw std::invalid_argument("linear_dynamic_grouped_conv_add: not supported on r9700");
}

void linear_dynamic_grouped_conv(const Tensor& x, const Weight& projection_weight,
                                 const Tensor& base_kernel, const Tensor& finish_delta,
                                 Tensor& partial, CudaExecutionView execution) {
    throw std::invalid_argument("linear_dynamic_grouped_conv: not supported on r9700");
}

} // namespace ninfer::ops
