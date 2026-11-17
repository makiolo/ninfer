#include "ops/linear_attention/gated_delta_net/control/execute.h"
#include "ops/common/cuda/reduce.cuh"
#include "ops/common/host/launch.h"
#include "ops/common/numeric/math.cuh"
#include "core/device.h"
#include <cuda_bf16.h>

namespace ninfer::ops::detail::gdn_control {
namespace {
// A CTA owns one complete control row. In the norm form it consumes FP32 n directly;
// only the separately observable normalized output is rounded to BF16.
template <bool Normalize>
__global__ __launch_bounds__(256) void projection_kernel(
    const __nv_bfloat16* x, const __nv_bfloat16* a, const __nv_bfloat16* b,
    const __nv_bfloat16* gain, float eps, __nv_bfloat16* normalized, const float* a_log,
    const float* bias, float* g, float* beta, int heads, int hidden, int token_begin) {
    const int row = blockIdx.x, token = token_begin + blockIdx.y;
    const bool is_b    = row >= heads;
    const int head     = is_b ? row - heads : row;
    const auto* input  = x + std::int64_t(token) * hidden;
    const auto* weight = (is_b ? b : a) + std::int64_t(head) * hidden;
    __shared__ float partial[256 / common::kWarpSize];
    __shared__ float inverse;
    if constexpr (Normalize) {
        float ss = 0;
        for (int k = threadIdx.x; k < hidden; k += blockDim.x) {
            const float value = __bfloat162float(input[k]);
            ss                = fmaf(value, value, ss);
        }
        ss = common::block_reduce_sum<256>(ss, partial);
        if (threadIdx.x == 0) inverse = rsqrtf(ss / hidden + eps);
        __syncthreads();
    }
    float sum = 0;
    for (int k = threadIdx.x; k < hidden; k += blockDim.x) {
        float value = __bfloat162float(input[k]);
        if constexpr (Normalize) {
            value = (value * inverse) * (1.0f + __bfloat162float(gain[k]));
            if (row == 0) normalized[std::int64_t(token) * hidden + k] = __float2bfloat16_rn(value);
        }
        sum = fmaf(__bfloat162float(weight[k]), value, sum);
    }
    sum = common::block_reduce_sum<256>(sum, partial);
    if (threadIdx.x == 0) {
        const auto offset = std::int64_t(token) * heads + head;
        if (is_b)
            beta[offset] = common::sigmoid(sum);
        else
            g[offset] = -expf(a_log[head]) * common::softplus(sum + bias[head]);
    }
}

template <bool Normalize>
void launch(const Tensor& x, const Weights& weights, const Tensor& a_log, const Tensor& bias,
            const Tensor& gain, float eps, Tensor normalized, GdnControlOutputs out,
            CudaExecutionView execution) {
    const auto [heads, hidden] = weights.geometry;
    const auto* a              = static_cast<const __nv_bfloat16*>(weights.first->qdata);
    const auto* b = weights.second ? static_cast<const __nv_bfloat16*>(weights.second->qdata)
                                   : a + std::int64_t(heads) * hidden;
    common::for_each_token_slice(x.ne[1], 1, [&](int begin, int count) {
        projection_kernel<Normalize><<<dim3(2 * heads, count), 256, 0, execution.stream()>>>(
            static_cast<const __nv_bfloat16*>(x.data), a, b,
            static_cast<const __nv_bfloat16*>(gain.data), eps,
            static_cast<__nv_bfloat16*>(normalized.data), static_cast<const float*>(a_log.data),
            static_cast<const float*>(bias.data), static_cast<float*>(out.g.data),
            static_cast<float*>(out.beta.data), heads, hidden, begin);
        CUDA_CHECK(cudaGetLastError());
    });
}
} // namespace

void execute_projection(const Tensor& x, const Weights& weights, const Tensor& a_log,
                        const Tensor& dt_bias, GdnControlOutputs out, Plan, Scratch,
                        CudaExecutionView execution) {
    launch<false>(x, weights, a_log, dt_bias, {}, 0, {}, out, execution);
}

void execute_norm(const Tensor& x, const Tensor& norm_weight, float eps, const Weights& weights,
                  const Tensor& a_log, const Tensor& dt_bias, GdnNormControlOutputs out, NormPlan,
                  Scratch, CudaExecutionView execution) {
    launch<true>(x, weights, a_log, dt_bias, norm_weight, eps, out.normalized, out.control,
                 execution);
}
} // namespace ninfer::ops::detail::gdn_control
