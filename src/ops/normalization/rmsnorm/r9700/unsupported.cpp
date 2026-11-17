#include "ninfer/ops/rmsnorm_pack_tail.h"

#include <stdexcept>

namespace ninfer::ops {

void rmsnorm_pack_tail(const Tensor& input, const Tensor& weight, float epsilon, Tensor& output,
                       CudaExecutionView execution) {
    throw std::invalid_argument("rmsnorm_pack_tail: not supported on r9700");
}

} // namespace ninfer::ops
