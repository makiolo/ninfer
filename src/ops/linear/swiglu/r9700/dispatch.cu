#include "ops/linear/swiglu/dispatch.h"
#include "ops/linear/swiglu/fp8/dispatch.h"
#include "ops/linear/swiglu/nvfp4/dispatch.h"
#include "ops/linear/swiglu/mxfp8/dispatch.h"
#include "ops/linear/swiglu/fp8_block/dispatch.h"
#include "ops/linear/swiglu/nvfp4_mul/dispatch.h"
#include "ops/linear/common/operands.h"
#include "ops/linear/kernels/q4/q4_operands.h"
#include "ops/linear/kernels/common/r9700/rowsplit.cuh"
#include "ops/common/numeric/math.cuh"

#include <stdexcept>

namespace ninfer::ops::detail {
namespace {
struct SwiGluEpilogue {
    __device__ __forceinline__ float apply_pair(int, int, float gate, float up) const {
        // Preserve the negative tail (including gate=-60); no tanh sigmoid or
        // intermediate BF16 boundary between either contraction and the product.
        return common::silu(gate) * up;
    }
};

[[noreturn]] void unsupported() {
    throw std::invalid_argument("linear_swiglu: r9700 supports only unclamped Q4 [34816,5120]");
}
} // namespace

void validate_swiglu_geometry(QType type, int rows, int k, bool clamped) {
    if (type != QType::Q4_G64_FP16 || rows != 34816 || k != 5120 || clamped) unsupported();
}

std::size_t q4_linear_swiglu_capacity_workspace_bytes(int rows, int k, int first, int last) {
    validate_swiglu_geometry(QType::Q4_G64_FP16, rows, k);
    if (first <= 0 || last < first)
        throw std::invalid_argument("linear_swiglu: invalid token interval");
    return 0;
}

void q4_linear_swiglu_dispatch(const Tensor& x, const Weight& weight, Tensor& out, MemoryArena&,
                               CudaExecutionView execution) {
    validate_swiglu_geometry(weight.qtype, weight.n, weight.k);
    validate_projection_operands(x, weight, {&out, 1}, nullptr, execution, "linear_swiglu");
    if (weight.qhigh || weight.high_plane_bytes)
        throw std::invalid_argument("linear_swiglu: Q4 must not have a high plane");
    namespace operand = common::operand;
    const auto output_range = operand::range(out, "linear_swiglu");
    const auto elements     = std::size_t(weight.n) * weight.k;
    operand::disjoint(output_range, operand::range(weight.qdata, elements / 2, "linear_swiglu"),
                      "linear_swiglu");
    operand::disjoint(output_range, operand::range(weight.scales, elements / 64 * 2,
                                                  "linear_swiglu"), "linear_swiglu");
    const auto operands = q4_linear_operands(x, weight);
    validate_q4_operands(operands);
    const LinearBf16Output output{static_cast<__nv_bfloat16*>(out.data), weight.n / 2};
    if (x.ne[1] == 1)
        launch_rowsplit_a16<4, 1, true>(operands, output, SwiGluEpilogue{}, execution.stream());
    else
        launch_rowsplit_a16<4, 8, true>(operands, output, SwiGluEpilogue{}, execution.stream());
}

// The shared public entry reaches this target's geometry check before dispatch.
// Unsupported bindings remain explicit and never link CUDA/PTX implementations.
void q8_linear_swiglu_dispatch(const Tensor&, const Weight&, Tensor&, float, bool, cudaStream_t) {
    unsupported();
}
std::size_t fp8_linear_swiglu_workspace_capacity_bytes(LinearPolicy, std::int32_t, std::int32_t) {
    unsupported();
}
std::size_t nvfp4_linear_swiglu_workspace_capacity_bytes(LinearPolicy, std::int32_t, std::int32_t) {
    unsupported();
}
void fp8_linear_swiglu_dispatch(const Tensor&, const Weight&, Tensor&, LinearPolicy, MemoryArena&,
                                cudaStream_t) {
    unsupported();
}
void nvfp4_linear_swiglu_dispatch(const Tensor&, const Weight&, Tensor&, LinearPolicy, MemoryArena&,
                                  cudaStream_t) {
    unsupported();
}
std::size_t mxfp8_swiglu_workspace_capacity_bytes(int, int, LinearPolicy, int, int) {
    unsupported();
}
std::size_t fp8_block_swiglu_workspace_capacity_bytes(int, int, LinearPolicy, int, int) {
    unsupported();
}
std::size_t nvfp4_mul_swiglu_workspace_capacity_bytes(int, int, LinearPolicy, int, int) {
    unsupported();
}
void mxfp8_swiglu_dispatch(const Tensor&, const Weight&, Tensor&, float, bool, LinearPolicy,
                           MemoryArena&, cudaStream_t) {
    unsupported();
}
void fp8_block_swiglu_dispatch(const Tensor&, const Weight&, Tensor&, float, bool, LinearPolicy,
                               cudaStream_t) {
    unsupported();
}
void nvfp4_mul_swiglu_dispatch(const Tensor&, const Weight&, const Tensor*, Tensor&, float, bool,
                               LinearPolicy, cudaStream_t) {
    unsupported();
}
} // namespace ninfer::ops::detail
