#pragma once

#include "core/device.h"
#include "ops/common/cuda/reduce.cuh"
#include "ops/common/quant/fp8.cuh"
#include "ops/common/quant/integer.cuh"
#include "ops/linear/common/epilogue.cuh"
#include <cuda_fp16.h>
#include <hip/hip_fp4.h>

namespace ninfer::ops::detail::gdn_input {
enum class Packed { Q4, Q5, Fp8, Nvfp4 };

struct PackedWeight {
    const std::uint8_t* codes;
    const std::uint8_t* high;
    const std::uint8_t* scales;
    int rows, hidden;
    float alpha;

    template <Packed Format>
    __device__ __forceinline__ float value(int row, int k) const {
        const auto i = std::int64_t(row) * hidden + k;
        if constexpr (Format == Packed::Fp8) {
            return common::decode_e4m3(codes[i]) *
                   __bfloat162float(reinterpret_cast<const __nv_bfloat16*>(scales)[row]);
        } else if constexpr (Format == Packed::Nvfp4) {
            __hip_fp4_e2m1 code;
            code.__x        = (codes[i / 2] >> ((i & 1) * 4)) & 15;
            const int group = k / 16;
            // BlockScaleK16M128x4 storage; the SDK supplies E2M1 widening on gfx1201.
            const auto offset = std::int64_t((row / 128) * (hidden / 64) + group / 4) * 512 +
                                (row & 31) * 16 + ((row & 127) >> 5) * 4 + (group & 3);
            return static_cast<float>(code) * common::decode_e4m3(scales[offset]);
        } else {
            float lo, hi;
            if constexpr (Format == Packed::Q5)
                common::Q5ScalarDecodeAtom::load_pair(codes, high, scales, i / 64, (k % 64) / 2, lo,
                                                      hi);
            else
                common::Q4SimtDecodeAtom::decode_pair(
                    codes[i / 2], reinterpret_cast<const std::uint16_t*>(scales)[i / 64], lo, hi);
            return k & 1 ? hi : lo;
        }
    }
};

// Private contraction for the fused GDN consumers: a wave owns a complete row and
// retains all columns until its convolution publishes. No device allocation or repack.
template <Packed Format, int Tokens, class Output, class Epilogue>
__global__ __launch_bounds__(128) void project_kernel(const __nv_bfloat16* x, PackedWeight weight,
                                                      int tokens, Output output,
                                                      Epilogue epilogue) {
    const int lane = int(threadIdx.x) % common::kWarpSize;
    const int row  = int(blockIdx.x) * 4 + int(threadIdx.x) / common::kWarpSize;
    if (row >= weight.rows) return;
    float sums[Tokens]{};
    for (int k = lane; k < weight.hidden; k += common::kWarpSize) {
        const float w = weight.value<Format>(row, k);
#pragma unroll
        for (int token = 0; token < Tokens; ++token)
            if (token < tokens)
                sums[token] = fmaf(w, __bfloat162float(x[std::int64_t(token) * weight.hidden + k]),
                                   sums[token]);
    }
#pragma unroll
    for (int token = 0; token < Tokens; ++token) {
        sums[token] = common::warp_reduce_sum(sums[token]);
        if constexpr (Format == Packed::Nvfp4) sums[token] *= weight.alpha;
    }
    if (lane == 0) linear_finish_row(output, epilogue, row, 0, sums, tokens);
}

template <Packed Format, int Tokens, class Output, class Epilogue>
void launch_packed(const Tensor& x, const Weight& weight, Output output, Epilogue epilogue,
                   cudaStream_t stream) {
    const PackedWeight packed{static_cast<const std::uint8_t*>(weight.qdata),
                              static_cast<const std::uint8_t*>(weight.qhigh),
                              static_cast<const std::uint8_t*>(weight.scales),
                              weight.n,
                              weight.k,
                              Format == Packed::Nvfp4 ? 1.0f / weight.weight_scale : 1.0f};
    project_kernel<Format, Tokens><<<(weight.n + 3) / 4, 128, 0, stream>>>(
        static_cast<const __nv_bfloat16*>(x.data), packed, x.ne[1], output, epilogue);
    CUDA_CHECK(cudaGetLastError());
}
} // namespace ninfer::ops::detail::gdn_input
