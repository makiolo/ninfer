#pragma once

#include "ops/linear/kernels/q4/q4_launch.h"

namespace ninfer::ops::detail {
void launch_q4_r9700_gemv(const Tensor&, const Weight&, Tensor&, cudaStream_t);
void launch_q4_r9700_simt(const Tensor&, const Weight&, Tensor&, cudaStream_t);
} // namespace ninfer::ops::detail
