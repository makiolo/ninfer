#include "ops/linear/projection/q4/q4_shapes.h"
#include "ops/linear/kernels/q4/r9700/launch.h"

namespace ninfer::ops::detail {

// Preserve each geometry's public token domain. r9700 uses only direct
// GEMV/SIMT instances; CUDA MMA/TMA/sliced-K instances are not linked here.
Q4Launch select_q4_n1024_k5120(std::int32_t tokens) {
    return tokens == 1 ? launch_q4_r9700_gemv : launch_q4_r9700_simt;
}

Q4Launch select_q4_n4096_k5120(std::int32_t tokens) {
    return tokens == 1 ? launch_q4_r9700_gemv : launch_q4_r9700_simt;
}

Q4Launch select_q4_n5120_k6144(std::int32_t tokens) {
    return tokens == 1 ? launch_q4_r9700_gemv : launch_q4_r9700_simt;
}

Q4Launch select_q4_n6144_k5120(std::int32_t tokens) {
    return tokens == 1 ? launch_q4_r9700_gemv : launch_q4_r9700_simt;
}

Q4Launch select_q4_n7168_k5120(std::int32_t tokens) {
    return tokens == 1 ? launch_q4_r9700_gemv : launch_q4_r9700_simt;
}

Q4Launch select_q4_n34816_k5120(std::int32_t tokens) {
    return tokens == 1 ? launch_q4_r9700_gemv : launch_q4_r9700_simt;
}

Q4Launch select_q4_n131072_k5120(std::int32_t tokens) {
    return tokens == 1 ? launch_q4_r9700_gemv : launch_q4_r9700_simt;
}

Q4Launch select_q4_n131072_k2048(std::int32_t tokens) {
    return tokens == 1 ? launch_q4_r9700_gemv : launch_q4_r9700_simt;
}

Q4Launch select_q4_n248320_k2560(std::int32_t tokens) {
    return tokens == 1 ? launch_q4_r9700_gemv : launch_q4_r9700_simt;
}

Q4Launch select_q4_n124160_k2560(std::int32_t tokens) {
    return tokens == 1 ? launch_q4_r9700_gemv : launch_q4_r9700_simt;
}

} // namespace ninfer::ops::detail
