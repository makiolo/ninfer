#pragma once
#include "core/platform.h"
#include "ops/common/cuda/reduce.cuh"
#include "ops/normalization/rmsnorm/kernel/epilogue.cuh"
#include <cuda_bf16.h>
#include <cstdint>

namespace ninfer::ops::detail::rmsnorm {

// Complete scalar route for odd dimensions and backing without BF16x2 alignment.
template <Epilogue Kind, bool SaveInverse = false>
__launch_bounds__(256) __global__
    void rmsnorm_generic_kernel(const __nv_bfloat16* x, const __nv_bfloat16* weight,
                                const __nv_bfloat16* z, __nv_bfloat16* out, std::int32_t d,
                                std::int64_t rows, float eps, float* inverse_output) {
    const std::int64_t row = static_cast<std::int64_t>(blockIdx.x);
    if (row >= rows) { return; }

    const std::int64_t base = row * static_cast<std::int64_t>(d);
#if NINFER_TARGET_R9700
    // Scale before squaring: neither a large row sum nor tiny squares should
    // change the represented-input formula. Block collectives belong to common.
    float maximum = 0.0f;
    for (std::int64_t i = threadIdx.x; i < d; i += blockDim.x)
        maximum = fmaxf(maximum, fabsf(__bfloat162float(x[base + i])));
    const double scale = static_cast<double>(common::block_max_broadcast<256>(maximum));
    float sum = 0.0f;
    for (std::int64_t i = threadIdx.x; i < d; i += blockDim.x) {
        const float value = scale == 0.0 ? 0.0f :
            static_cast<float>(static_cast<double>(__bfloat162float(x[base + i])) / scale);
        sum += value * value;
    }
    __shared__ float warp_sums[256 / common::kWarpSize];
    __shared__ float inverse;
    const float total = common::block_reduce_sum<256>(sum, warp_sums);
    if (threadIdx.x == 0)
        inverse = static_cast<float>(1.0 / sqrt(scale * scale *
            (static_cast<double>(total) / d) + static_cast<double>(eps)));
    __syncthreads();
    const float inv = inverse;
#else
    float sum               = 0.0f;
    for (std::int64_t i = threadIdx.x; i < static_cast<std::int64_t>(d); i += blockDim.x) {
        const float xv = __bfloat162float(x[base + i]);
        sum += xv * xv;
    }

    __shared__ float scratch[256];
    scratch[threadIdx.x] = sum;
    __syncthreads();
    for (int stride = blockDim.x / 2; stride > 0; stride >>= 1) {
        if (threadIdx.x < stride) { scratch[threadIdx.x] += scratch[threadIdx.x + stride]; }
        __syncthreads();
    }

    const float inv = rsqrtf(scratch[0] / static_cast<float>(d) + eps);
#endif
    if constexpr (SaveInverse) {
        if (threadIdx.x == 0) inverse_output[row] = inv;
    }
    for (std::int64_t i = threadIdx.x; i < static_cast<std::int64_t>(d); i += blockDim.x) {
        const std::int64_t index = base + i;
        const float xv           = __bfloat162float(x[index]);
        const float wv           = __bfloat162float(weight[i]);
        float zv                 = 0.0f;
        if constexpr (is_gated<Kind>) { zv = __bfloat162float(z[index]); }
        out[index] = __float2bfloat16_rn(apply_epilogue<Kind>(xv, inv, wv, zv));
    }
}

} // namespace ninfer::ops::detail::rmsnorm
