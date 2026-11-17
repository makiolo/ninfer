#include "ops/linear/kernels/q4/r9700/launch.h"
#include "ops/linear/kernels/q4/q4_operands.h"
#include "ops/linear/kernels/common/r9700/rowsplit.cuh"

namespace ninfer::ops::detail {
void launch_q4_r9700_gemv(const Tensor& x, const Weight& w, Tensor& out, cudaStream_t stream) {
    const auto operands = q4_linear_operands(x, w);
    validate_q4_operands(operands);
    launch_rowsplit_a16<4, 1>(operands, out, stream);
}

void launch_q4_r9700_simt(const Tensor& x, const Weight& w, Tensor& out, cudaStream_t stream) {
    const auto operands = q4_linear_operands(x, w);
    validate_q4_operands(operands);
    launch_rowsplit_a16<4, 8>(operands, out, stream);
}
} // namespace ninfer::ops::detail
