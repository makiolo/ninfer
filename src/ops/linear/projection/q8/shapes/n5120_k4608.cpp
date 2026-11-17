#include "core/platform.h"
#if NINFER_TARGET_R9700
#include "ops/linear/kernels/q8/r9700/launch.h"
#endif
#include "ops/linear/projection/q8/shapes.h"

#include <stdexcept>

namespace ninfer::ops::detail {

Q8Launch select_q8_n5120_k4608(std::int32_t tokens) {
    if (tokens > 32768) throw std::invalid_argument("q8 linear: column extent exceeds 32768");
#if NINFER_TARGET_R9700
    return tokens == 1 ? launch_q8_r9700_gemv : launch_q8_r9700_simt;
#else
    if (tokens <= 4) return launch_q8_a16_simt_r8_t4;
    if (tokens <= 5) return launch_q8_a16_simt_r8_t8;
    return launch_q8_a16_mma_r64_t128;
#endif
}

} // namespace ninfer::ops::detail
