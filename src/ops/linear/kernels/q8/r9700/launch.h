#pragma once

#include "ops/linear/kernels/q8/q8_launch.h"

namespace ninfer::ops::detail {
void launch_q8_r9700_gemv(const Tensor&, const Weight&, Tensor&, cudaStream_t);
void launch_q8_r9700_simt(const Tensor&, const Weight&, Tensor&, cudaStream_t);
} // namespace ninfer::ops::detail
