#include "ninfer/ops/linear_bias.h"

#include <stdexcept>

namespace ninfer::ops {

void linear_bias(const Tensor& x, const Weight& weight, const Tensor& bias, Tensor& out,
                 LinearPolicy policy, MemoryArena& workspace, CudaExecutionView execution) {
    throw std::invalid_argument("linear_bias: not supported on r9700");
}

void linear_bias(const Tensor& x, const Weight& weight, const Tensor& bias, Tensor& out,
                 CudaExecutionView execution) {
    throw std::invalid_argument("linear_bias: not supported on r9700");
}

std::size_t linear_bias_workspace_capacity_bytes(QType qtype, std::int32_t rows,
                                                 std::int32_t input_rows, LinearPolicy policy,
                                                 std::int32_t min_tokens, std::int32_t max_tokens) {
    throw std::invalid_argument("linear_bias_workspace_capacity_bytes: not supported on r9700");
}

} // namespace ninfer::ops
