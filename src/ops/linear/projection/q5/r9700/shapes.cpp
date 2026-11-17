#include "ops/linear/projection/q5/q5_shapes.h"
#include "ops/linear/kernels/q5/r9700/launch.h"

namespace ninfer::ops::detail {

// Preserve each geometry's public token domain. r9700 uses only direct
// GEMV/SIMT instances; CUDA MMA/TMA/sliced-K instances are not linked here.
Q5Launch select_q5_n1024_k5120(std::int32_t tokens) {
    return tokens == 1 ? launch_q5_r9700_gemv : launch_q5_r9700_simt;
}

Q5Launch select_q5_n6144_k5120(std::int32_t tokens) {
    return tokens == 1 ? launch_q5_r9700_gemv : launch_q5_r9700_simt;
}

Q5Launch select_q5_n7168_k5120(std::int32_t tokens) {
    return tokens == 1 ? launch_q5_r9700_gemv : launch_q5_r9700_simt;
}

Q5Launch select_q5_n5120_k6144(std::int32_t tokens) {
    return tokens == 1 ? launch_q5_r9700_gemv : launch_q5_r9700_simt;
}

Q5Launch select_q5_n5120_k17408(std::int32_t tokens) {
    return tokens == 1 ? launch_q5_r9700_gemv : launch_q5_r9700_simt;
}

} // namespace ninfer::ops::detail
