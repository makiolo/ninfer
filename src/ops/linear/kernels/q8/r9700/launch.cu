#include "ops/linear/kernels/q8/r9700/launch.h"
#include "ops/linear/kernels/q8/q8_operands.h"
#include "ops/linear/kernels/common/r9700/rowsplit.cuh"

namespace ninfer::ops::detail {
void launch_q8_r9700_gemv(const Tensor& x, const Weight& w, Tensor& out, cudaStream_t stream) {
    const auto operands = q8_linear_operands(x, w);
    validate_q8_operands(operands);
    launch_rowsplit_a16<8, 1>(operands, out, stream);
}

void launch_q8_r9700_simt(const Tensor& x, const Weight& w, Tensor& out, cudaStream_t stream) {
    const auto operands = q8_linear_operands(x, w);
    validate_q8_operands(operands);
    launch_rowsplit_a16<8, 8>(operands, out, stream);
}
} // namespace ninfer::ops::detail
