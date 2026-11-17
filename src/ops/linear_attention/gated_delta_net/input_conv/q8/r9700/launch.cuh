#pragma once

#include "ops/linear/kernels/q8/q8_gemv_launch.cuh"

namespace ninfer::ops::detail::gdn_input {
// The contraction already supports a compile-time token capacity. A complete replay
// window belongs to one row consumer, so convolution can publish without scratch.
template <int Tokens>
struct Q8WindowSchedule : Q8A16GemvSchedule<4, 1, 1, 0, 1> {
    static constexpr int kBlockTokens = Tokens;
};

template <int Tokens, class Output, class Epilogue>
void launch_q8_window(const Tensor& x, const Weight& weight, Output output, Epilogue epilogue,
                      cudaStream_t stream) {
    launch_q8_a16_gemv<Q8WindowSchedule<Tokens>>(q8_linear_operands(x, weight), output, epilogue,
                                                 stream);
}
} // namespace ninfer::ops::detail::gdn_input
