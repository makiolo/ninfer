#include "ops/linear/projection/q8/shapes.h"
#include "ops/linear/kernels/q8/r9700/launch.h"

namespace ninfer::ops::detail {

// Preserve each geometry's public token domain. r9700 uses only direct
// GEMV/SIMT instances; CUDA MMA/TMA/sliced-K instances are not linked here.
Q8Launch select_q8_n1024_k2048(std::int32_t tokens) {
    return tokens == 1 ? launch_q8_r9700_gemv : launch_q8_r9700_simt;
}

Q8Launch select_q8_n1024_k5120(std::int32_t tokens) {
    return tokens == 1 ? launch_q8_r9700_gemv : launch_q8_r9700_simt;
}

Q8Launch select_q8_n2048_k4096(std::int32_t tokens) {
    return tokens == 1 ? launch_q8_r9700_gemv : launch_q8_r9700_simt;
}

Q8Launch select_q8_n2048_k16384(std::int32_t tokens) {
    return tokens == 1 ? launch_q8_r9700_gemv : launch_q8_r9700_simt;
}

Q8Launch select_q8_n2560_k6144(std::int32_t tokens) {
    return tokens == 1 ? launch_q8_r9700_gemv : launch_q8_r9700_simt;
}

Q8Launch select_q8_n5120_k6144(std::int32_t tokens) {
    return tokens == 1 ? launch_q8_r9700_gemv : launch_q8_r9700_simt;
}

Q8Launch select_q8_n5120_k10240(std::int32_t tokens) {
    return tokens == 1 ? launch_q8_r9700_gemv : launch_q8_r9700_simt;
}

Q8Launch select_q8_n5120_k17408(std::int32_t tokens) {
    return tokens == 1 ? launch_q8_r9700_gemv : launch_q8_r9700_simt;
}

Q8Launch select_q8_n5120_k25600(std::int32_t tokens) {
    return tokens == 1 ? launch_q8_r9700_gemv : launch_q8_r9700_simt;
}

Q8Launch select_q8_n6144_k2560(std::int32_t tokens) {
    return tokens == 1 ? launch_q8_r9700_gemv : launch_q8_r9700_simt;
}

Q8Launch select_q8_n6144_k5120(std::int32_t tokens) {
    return tokens == 1 ? launch_q8_r9700_gemv : launch_q8_r9700_simt;
}

Q8Launch select_q8_n9216_k2048(std::int32_t tokens) {
    return tokens == 1 ? launch_q8_r9700_gemv : launch_q8_r9700_simt;
}

Q8Launch select_q8_n10240_k2560(std::int32_t tokens) {
    return tokens == 1 ? launch_q8_r9700_gemv : launch_q8_r9700_simt;
}

Q8Launch select_q8_n12288_k2048(std::int32_t tokens) {
    return tokens == 1 ? launch_q8_r9700_gemv : launch_q8_r9700_simt;
}

Q8Launch select_q8_n12288_k2560(std::int32_t tokens) {
    return tokens == 1 ? launch_q8_r9700_gemv : launch_q8_r9700_simt;
}

Q8Launch select_q8_n14336_k5120(std::int32_t tokens) {
    return tokens == 1 ? launch_q8_r9700_gemv : launch_q8_r9700_simt;
}

Q8Launch select_q8_n16384_k2560(std::int32_t tokens) {
    return tokens == 1 ? launch_q8_r9700_gemv : launch_q8_r9700_simt;
}

Q8Launch select_q8_n34816_k5120(std::int32_t tokens) {
    return tokens == 1 ? launch_q8_r9700_gemv : launch_q8_r9700_simt;
}

Q8Launch select_q8_n248320_k5120(std::int32_t tokens) {
    return tokens == 1 ? launch_q8_r9700_gemv : launch_q8_r9700_simt;
}

} // namespace ninfer::ops::detail
