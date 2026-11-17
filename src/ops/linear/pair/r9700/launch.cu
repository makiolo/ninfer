#include "ops/linear/pair/dispatch.h"
#include "ops/linear/kernels/q8/q8_operands.h"
#include "ops/linear/kernels/common/r9700/rowsplit.cuh"

#include <stdexcept>

namespace ninfer::ops::detail {
namespace {
void launch(const Tensor& x, const Weight& first, const Weight& second, Tensor& first_out,
            Tensor& second_out, cudaStream_t stream) {
    const auto first_operands  = q8_linear_operands(x, first);
    const auto second_operands = q8_linear_operands(x, second);
    validate_q8_operands(first_operands);
    validate_q8_operands(second_operands);
    if (x.ne[1] == 1) {
        launch_rowsplit_a16<8, 1>(first_operands, first_out, stream);
        launch_rowsplit_a16<8, 1>(second_operands, second_out, stream);
    } else {
        launch_rowsplit_a16<8, 8>(first_operands, first_out, stream);
        launch_rowsplit_a16<8, 8>(second_operands, second_out, stream);
    }
}
} // namespace

LinearPairLaunch select_q8_pair_n1024_k2048(int tokens) {
    if (tokens <= 0) throw std::invalid_argument("linear_pair: nonpositive token extent");
    return launch;
}

LinearPairLaunch select_q8_pair_n1024_k5120(int tokens) {
    if (tokens <= 0) throw std::invalid_argument("linear_pair: nonpositive token extent");
    return launch;
}

void launch_bf16_pair(const Tensor&, const Tensor&, const Weight&, const Weight&, Tensor&, Tensor&,
                      cudaStream_t) {
    throw std::invalid_argument("linear_pair: BF16 is not supported on r9700");
}
} // namespace ninfer::ops::detail
