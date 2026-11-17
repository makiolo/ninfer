#pragma once

#include "ops/common/cuda/reduce.cuh"
#include "ops/common/quant/fp8.cuh"
#include "ops/kv_cache/kernels/hadamard.cuh"
#include "ops/kv_cache/kernels/layout.cuh"
#include "ops/softmax_attention/kernels/causal/operands.h"

namespace ninfer::ops::detail::r9700 {

template <bool Fp8, class Cache>
__device__ __forceinline__ float cache_component(Cache cache, bool key, std::int64_t record,
                                                int head, int position, int d) {
    if constexpr (Fp8) {
        const auto code = kv::Fp8G128Layout::code_offset(record, head, d, position & kv::kPageMask);
        const auto scale = kv::Fp8G128Layout::scale_offset(record, head, d / 128,
                                                          position & kv::kPageMask);
        return common::decode_e4m3((key ? cache.keys : cache.values)[code]) *
               common::ue8m0_to_float((key ? cache.key_scales : cache.value_scales)[scale]);
    } else {
        const auto offset = kv::record_offset<256, 2>(record, head, position & kv::kPageMask, d);
        return key ? __bfloat162float(cache.keys[offset]) : __half2float(cache.values[offset]);
    }
}

// A block owns one query/head/key partition. Four wave32s consume disjoint keys;
// each lane holds eight features. All softmax state stays FP32 until the final
// BF16 output. The persistent cache is already encoded by the preceding KV write.
template <bool Fp8, class Cache>
__global__ __launch_bounds__(128) void causal_partition(CausalAttentionOperands p, Cache cache,
                                                       int splits, CausalPartialView partial) {
    constexpr int Warps = 4;
    const int token = blockIdx.x, head = blockIdx.y;
    const int batch = blockIdx.z / splits, split = blockIdx.z % splits;
    const int lane = threadIdx.x & 31, warp = threadIdx.x >> 5;
    const bool live = !cache.valid_columns || token < cache.valid_columns[batch];
    const auto query = (static_cast<std::int64_t>(batch) * p.width + token) * p.query_heads + head;
    float numerator[8]{};
    float maximum = -CUDART_INF_F, denominator = 0.0f;

    // Inert columns must not read positions, table selectors or cache records.
    if (live) {
        float q[8];
#pragma unroll
        for (int r = 0; r < 8; ++r) q[r] = __bfloat162float(p.q[query * 256 + lane + 32 * r]);
        if constexpr (Fp8) kv::normalized_hadamard_d256_inplace(q, lane);
        const int visible = p.positions[static_cast<std::int64_t>(batch) * p.width + token] + 1;
        const int span = (visible + splits - 1) / splits;
        const int begin = split * span, end = min(visible, begin + span);
        const int kv_head = head / (p.query_heads / cache.kv_heads);
        const auto* table = cache.tables +
            (cache.table_rows ? static_cast<std::int64_t>(cache.table_rows[batch]) * cache.table_stride : 0);
        for (int key = begin + warp; key < end; key += Warps) {
            const auto record = kv::lookup_page(table, key);
            float dot = 0.0f;
#pragma unroll
            for (int r = 0; r < 8; ++r)
                dot = fmaf(q[r], cache_component<Fp8>(cache, true, record, kv_head, key,
                                                      lane + 32 * r), dot);
            const float score = common::warp_sum(dot) * p.scale;
            const float next = fmaxf(maximum, score);
            const float correction = expf(maximum - next);
            const float probability = expf(score - next);
            denominator = fmaf(denominator, correction, probability);
#pragma unroll
            for (int r = 0; r < 8; ++r)
                numerator[r] = fmaf(probability,
                    cache_component<Fp8>(cache, false, record, kv_head, key, lane + 32 * r),
                    numerator[r] * correction);
            maximum = next;
        }
    }

    __shared__ float warp_acc[Warps][256];
    __shared__ float warp_maximum[Warps], warp_sum[Warps];
#pragma unroll
    for (int r = 0; r < 8; ++r) warp_acc[warp][lane + 32 * r] = numerator[r];
    if (lane == 0) {
        warp_maximum[warp] = maximum;
        warp_sum[warp] = denominator;
    }
    __syncthreads();

    float block_maximum = -CUDART_INF_F;
#pragma unroll
    for (int w = 0; w < Warps; ++w) block_maximum = fmaxf(block_maximum, warp_maximum[w]);
    float weights[Warps], block_sum = 0.0f;
#pragma unroll
    for (int w = 0; w < Warps; ++w) {
        weights[w] = warp_sum[w] > 0.0f ? expf(warp_maximum[w] - block_maximum) : 0.0f;
        block_sum = fmaf(warp_sum[w], weights[w], block_sum);
    }
    const auto part = ((static_cast<std::int64_t>(batch) * splits + split) * p.width + token) *
                      p.query_heads + head;
    for (int d = threadIdx.x; d < 256; d += 128) {
        float value = 0.0f;
#pragma unroll
        for (int w = 0; w < Warps; ++w) value = fmaf(warp_acc[w][d], weights[w], value);
        if (splits == 1)
            p.out[query * 256 + d] = __float2bfloat16_rn(block_sum > 0.0f ? value / block_sum : 0.0f);
        else
            partial.acc[part * 256 + d] = value;
    }
    if (splits != 1 && threadIdx.x == 0) {
        partial.maximum[part] = block_maximum;
        partial.sum[part] = block_sum;
    }
}

__global__ __launch_bounds__(256) void causal_merge(CausalAttentionOperands p, int splits,
                                                   CausalPartialView partial) {
    const int token = blockIdx.x, head = blockIdx.y, batch = blockIdx.z, d = threadIdx.x;
    const auto stride = static_cast<std::int64_t>(p.width) * p.query_heads;
    const auto first = static_cast<std::int64_t>(batch) * splits * stride +
                       static_cast<std::int64_t>(token) * p.query_heads + head;
    // Every wave computes the same small scalar reduction. Empty partitions have
    // sum=0 and never evaluate exp(-inf - -inf).
    const int lane = d & 31;
    const float m = lane < splits ? partial.maximum[first + lane * stride] : -CUDART_INF_F;
    const float l = lane < splits ? partial.sum[first + lane * stride] : 0.0f;
    const float maximum = common::warp_max(m);
    const float weight = l > 0.0f ? expf(m - maximum) : 0.0f;
    const float denominator = common::warp_sum(l * weight);
    float value = 0.0f;
    for (int split = 0; split < splits; ++split) {
        const float multiplier = __shfl_sync(common::kFullWarpMask, weight, split);
        value = fmaf(partial.acc[(first + split * stride) * 256 + d], multiplier, value);
    }
    const auto query = (static_cast<std::int64_t>(batch) * p.width + token) * p.query_heads + head;
    p.out[query * 256 + d] = __float2bfloat16_rn(denominator > 0.0f ? value / denominator : 0.0f);
}

} // namespace ninfer::ops::detail::r9700
