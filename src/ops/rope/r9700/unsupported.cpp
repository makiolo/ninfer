#include "ninfer/ops/rmsnorm_rope.h"

#include <stdexcept>

namespace ninfer::ops {

void rmsnorm_rope(const Tensor& positions, const Tensor& q_norm_weight, const Tensor& k_norm_weight,
                  float epsilon, float theta, Tensor& q, Tensor& k, CudaExecutionView execution) {
    throw std::invalid_argument("rmsnorm_rope: not supported on r9700");
}

void rmsnorm_rope(const Tensor& positions, const Tensor& norm_weight, float epsilon, float theta,
                  Tensor& x, CudaExecutionView execution) {
    throw std::invalid_argument("rmsnorm_rope: not supported on r9700");
}

} // namespace ninfer::ops
