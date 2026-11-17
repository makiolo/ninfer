#pragma once

#include "core/device.h"
#include "core/tensor.h"
#include "ops/common/cuda/memory.cuh"
#include "ops/common/cuda/reduce.cuh"
#include "ops/common/host/launch.h"
#include "ops/linear/common/epilogue.cuh"

#include <cuda_fp16.h>

namespace ninfer::ops::detail {

// A complete row contraction within one wave32. Keep FP32 decoded weights,
// explicit fmaf accumulation and the same reduction tree for every consumer.
template <int Bits, int BlockTokens, class Operands>
__device__ __forceinline__ void rowsplit_a16_dot(const Operands& operands, int row,
                                                int token_begin, int token0, int active_tokens,
                                                float (&acc)[BlockTokens]) {
    static_assert(Bits == 4 || Bits == 5 || Bits == 8);
    static_assert(BlockTokens == 1 || BlockTokens == 8);
    constexpr int GroupK = Bits == 8 ? 32 : 64;
    const int lane       = int(threadIdx.x) & 31;
    const auto row_offset   = std::int64_t(row) * operands.padded_k;
    for (int kk = lane * 8; kk < operands.k; kk += 256) {
        const auto group      = (row_offset + kk) / GroupK;
        const auto scale_bits = common::load_vec<std::uint16_t>(operands.scales + group * 2);
        const float scale     = __half2float(__ushort_as_half(scale_bits));
        float weights[8];
        if constexpr (Bits == 8) {
            const uint2 codes = common::load_vec<uint2>(operands.codes + row_offset + kk);
#pragma unroll
            for (int i = 0; i < 8; ++i) {
                const unsigned word = i < 4 ? codes.x : codes.y;
                weights[i] = float(static_cast<std::int8_t>(word >> ((i & 3) * 8))) * scale;
            }
        } else {
            const auto codes =
                common::load_vec<std::uint32_t>(operands.codes + (row_offset + kk) / 2);
            unsigned high = 0;
            if constexpr (Bits == 5) high = operands.high[(row_offset + kk) / 8];
#pragma unroll
            for (int i = 0; i < 8; ++i) {
                const int code     = int((codes >> (4 * i)) & 15u) | int(((high >> i) & 1u) << 4);
                constexpr int sign = 1 << (Bits - 1);
                weights[i]         = float((code ^ sign) - sign) * scale;
            }
        }
#pragma unroll
        for (int token = 0; token < BlockTokens; ++token) {
            if (token < active_tokens) {
                const auto* x =
                    operands.x + std::int64_t(token_begin + token0 + token) * operands.k + kk;
                const auto values = common::load_vec<uint4>(x);
                const unsigned words[]{values.x, values.y, values.z, values.w};
#pragma unroll
                for (int i = 0; i < 8; ++i) {
                    const unsigned bits = (words[i / 2] >> ((i & 1) * 16)) & 0xffffu;
                    acc[token]          = fmaf(weights[i], __uint_as_float(bits << 16), acc[token]);
                }
            }
        }
    }
#pragma unroll
    for (int token = 0; token < BlockTokens; ++token)
        acc[token] = common::warp_reduce_sum(acc[token]);
}

// Direct row-split contraction, optionally consuming matching rows from both
// halves (gate/up) in the same warp. No intermediate BF16 rounding or scratch.
template <int Bits, int BlockTokens, bool PairHalves, class Operands, class Output, class Epilogue>
__global__ __launch_bounds__(128) void rowsplit_a16_kernel(Operands operands, Output output,
                                                         Epilogue epilogue, int token_begin,
                                                         int tokens) {
    const int lane = int(threadIdx.x) & 31;
    const int row  = int(blockIdx.x) * 4 + (int(threadIdx.x) >> 5);
    const int rows = PairHalves ? operands.rows / 2 : operands.rows;
    if (row >= rows) return; // Entire warp; no CTA synchronization.
    const int token0        = int(blockIdx.y) * BlockTokens;
    const int active_tokens = min(BlockTokens, tokens - token0);
    float acc[BlockTokens]{};
    rowsplit_a16_dot<Bits>(operands, row, token_begin, token0, active_tokens, acc);
    if constexpr (PairHalves) {
        float second[BlockTokens]{};
        rowsplit_a16_dot<Bits>(operands, row + rows, token_begin, token0, active_tokens, second);
        if (lane == 0) {
#pragma unroll
            for (int token = 0; token < BlockTokens; ++token)
                if (token < active_tokens) {
                    const int global_token = token_begin + token0 + token;
                    output.store(row, global_token,
                                 epilogue.apply_pair(row, global_token, acc[token], second[token]));
                }
        }
    } else if (lane == 0) {
        linear_finish_row(output, epilogue, row, token_begin + token0, acc, active_tokens);
    }
}

// The format launcher validates its operand planes. Vector loads require eight
// logical columns; in particular K=4304 reads its final 16 columns, not K padding.
template <int Bits, int BlockTokens, bool PairHalves = false, class Operands, class Output,
          class Epilogue>
void launch_rowsplit_a16(const Operands& operands, Output output, Epilogue epilogue,
                        cudaStream_t stream) {
    if (operands.k % 8 != 0) throw std::invalid_argument("row-split SIMT requires K aligned to 8");
    if constexpr (BlockTokens == 1) {
        if (operands.tokens != 1) throw std::invalid_argument("row-split GEMV requires T=1");
    }
    if constexpr (PairHalves)
        if (operands.rows % 2 != 0)
            throw std::invalid_argument("row-split paired halves require even rows");
    const int rows = PairHalves ? operands.rows / 2 : operands.rows;
    common::for_each_token_slice(operands.tokens, BlockTokens, [&](int offset, int count) {
        const dim3 grid((rows + 3) / 4, (count + BlockTokens - 1) / BlockTokens);
        rowsplit_a16_kernel<Bits, BlockTokens, PairHalves>
            <<<grid, 128, 0, stream>>>(operands, output, epilogue, offset, count);
        CUDA_CHECK(cudaGetLastError());
    });
}

// Projection retains its identity epilogue and output addressing.
template <int Bits, int BlockTokens, class Operands>
void launch_rowsplit_a16(const Operands& operands, Tensor& out, cudaStream_t stream) {
    launch_rowsplit_a16<Bits, BlockTokens>(
        operands, LinearBf16Output{static_cast<__nv_bfloat16*>(out.data), operands.rows},
        LinearIdentityEpilogue{}, stream);
}

} // namespace ninfer::ops::detail
