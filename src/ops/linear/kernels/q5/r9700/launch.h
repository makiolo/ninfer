#pragma once

#include "ops/linear/kernels/q5/q5_launch.h"

namespace ninfer::ops::detail {
void launch_q5_r9700_gemv(const Tensor&, const Weight&, Tensor&, cudaStream_t);
void launch_q5_r9700_simt(const Tensor&, const Weight&, Tensor&, cudaStream_t);
} // namespace ninfer::ops::detail
