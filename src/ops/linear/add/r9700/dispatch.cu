#include "ops/linear/add/dispatch.h"
#include "ops/linear/add/fp8/dispatch.h"
#include "ops/linear/add/nvfp4/dispatch.h"
#include "ops/linear/kernels/q5/q5_operands.h"
#include "ops/linear/kernels/common/r9700/rowsplit.cuh"
#include "ops/common/host/operands.h"

#include <stdexcept>

namespace ninfer::ops::detail {
namespace {
void launch(const Tensor& x, const Weight& weight, Tensor& residual, cudaStream_t stream) {
    if (weight.ndim != 2 || weight.shape[0] != weight.n || weight.shape[1] != weight.k)
        throw std::invalid_argument("linear_add: invalid Q5 matrix geometry");
    namespace operand = common::operand;
    const auto range  = operand::range(residual, "linear_add");
    const auto elements = std::size_t(weight.n) * weight.k;
    operand::disjoint(range, operand::range(weight.qdata, elements / 2, "linear_add"),
                      "linear_add");
    operand::disjoint(range, operand::range(weight.qhigh, elements / 8, "linear_add"),
                      "linear_add");
    operand::disjoint(range, operand::range(weight.scales, elements / 64 * 2, "linear_add"),
                      "linear_add");
    if (weight.payload)
        operand::disjoint(range, operand::range(weight.payload, weight.payload_bytes, "linear_add"),
                          "linear_add");
    const auto operands = q5_linear_operands(x, weight);
    validate_q5_operands(operands);
    const LinearBf16Output output{static_cast<__nv_bfloat16*>(residual.data), weight.n};
    const LinearResidualAddEpilogue epilogue{{output.data, weight.n}};
    if (x.ne[1] == 1)
        launch_rowsplit_a16<5, 1>(operands, output, epilogue, stream);
    else
        launch_rowsplit_a16<5, 8>(operands, output, epilogue, stream);
}

[[noreturn]] void unsupported() {
    throw std::invalid_argument("linear_add: r9700 supports only Q5 [5120,6144/17408]");
}
} // namespace

LinearAddLaunch select_q5_linear_add(std::int32_t rows, std::int32_t k, std::int32_t tokens) {
    if (rows == 5120 && (k == 6144 || k == 17408) && tokens > 0) return launch;
    unsupported();
}

LinearAddLaunch select_bf16_linear_add(std::int32_t, std::int32_t, std::int32_t) { unsupported(); }
LinearAddLaunch select_q4_linear_add(std::int32_t, std::int32_t, std::int32_t) { unsupported(); }
LinearAddLaunch select_q8_linear_add(std::int32_t, std::int32_t, std::int32_t) { unsupported(); }

std::size_t fp8_linear_add_workspace_capacity_bytes(std::int32_t, std::int32_t, LinearPolicy,
                                                    std::int32_t, std::int32_t) {
    unsupported();
}
std::size_t nvfp4_linear_add_workspace_capacity_bytes(std::int32_t, std::int32_t, LinearPolicy,
                                                      std::int32_t, std::int32_t) {
    unsupported();
}
void fp8_linear_add_dispatch(const Tensor&, const Weight&, Tensor&, LinearPolicy, MemoryArena&,
                             cudaStream_t) {
    unsupported();
}
void nvfp4_linear_add_dispatch(const Tensor&, const Weight&, Tensor&, LinearPolicy, MemoryArena&,
                               cudaStream_t) {
    unsupported();
}
} // namespace ninfer::ops::detail
