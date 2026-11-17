#include "ops/linear/projection/bf16/bf16_dispatch.h"
#include "ops/linear/projection/q6/q6_dispatch.h"
#include "ops/linear/projection/fp8/fp8_dispatch.h"
#include "ops/linear/projection/nvfp4/nvfp4_dispatch.h"
#include "ops/linear/projection/mxfp8/dispatch.h"
#include "ops/linear/projection/fp8_block/dispatch.h"
#include "ops/linear/projection/nvfp4_mul/dispatch.h"

#include <stdexcept>
#include <string>

namespace ninfer::ops::detail {
namespace {
[[noreturn]] void unsupported(const char* format) {
    throw std::invalid_argument(std::string("linear: ") + format + " is not supported on r9700");
}
} // namespace

// These are the r9700 implementations of the unregistered representation
// selectors. Both public call forms and workspace queries retain the shared
// entry's validation and reach the same explicit rejection, before any launch.
Bf16Launch select_bf16_launch(int, int, int, LinearPolicy) { unsupported("BF16"); }

Q6Launch select_q6_launch(int, int, int, LinearPolicy) { unsupported("Q6"); }

void bf16_dispatch(const Tensor&, const Weight&, Tensor&, LinearPolicy, cudaStream_t) {
    unsupported("BF16");
}

void q6_dispatch(const Tensor&, const Weight&, Tensor&, LinearPolicy, cudaStream_t) {
    unsupported("Q6");
}

void fp8_dispatch(const Tensor&, const Weight&, Tensor&, LinearPolicy, MemoryArena*, cudaStream_t) {
    unsupported("FP8");
}

void nvfp4_dispatch(const Tensor&, const Weight&, Tensor&, LinearPolicy, MemoryArena*,
                    cudaStream_t) {
    unsupported("NVFP4");
}

void mxfp8_dispatch(const Tensor&, const Weight&, Tensor&, LinearPolicy, MemoryArena*,
                    cudaStream_t) {
    unsupported("MXFP8");
}

void fp8_block_dispatch(const Tensor&, const Weight&, Tensor&, LinearPolicy, MemoryArena*,
                        cudaStream_t) {
    unsupported("FP8 block");
}

void nvfp4_mul_dispatch(const Tensor&, const Weight&, Tensor&, LinearPolicy, cudaStream_t) {
    unsupported("NVFP4 multiply");
}

std::size_t fp8_linear_workspace_capacity_bytes(int, int, LinearPolicy, int, int) {
    unsupported("FP8");
}

std::size_t nvfp4_linear_workspace_capacity_bytes(int, int, LinearPolicy, int, int) {
    unsupported("NVFP4");
}

std::size_t mxfp8_linear_workspace_capacity_bytes(int, int, LinearPolicy, int, int) {
    unsupported("MXFP8");
}

std::size_t fp8_block_linear_workspace_capacity_bytes(int, int, LinearPolicy, int, int) {
    unsupported("FP8 block");
}

std::size_t nvfp4_mul_linear_workspace_capacity_bytes(int, int, LinearPolicy, int, int) {
    unsupported("NVFP4 multiply");
}
} // namespace ninfer::ops::detail
