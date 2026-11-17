#include "ops/linear_attention/gated_delta_net/input_conv/execute.h"
#include "ops/linear_attention/gated_delta_net/input_conv/workspace.h"
#include "ops/common/host/operands.h"
#include "ops/linear_attention/gated_delta_net/input_conv/projected_conv.h"
#include "ops/linear_attention/gated_delta_net/input_conv/kernels/conv.cuh"
#include "ops/linear_attention/gated_delta_net/input_conv/r9700/project.cuh"
#include "ops/linear_attention/gated_delta_net/input_conv/q8/r9700/launch.cuh"
#include <algorithm>
#include <stdexcept>

namespace ninfer::ops::detail::gdn_input {
namespace {
struct SplitOutput {
    __nv_bfloat16* qkv;
    __nv_bfloat16* z;
    int channels, value_rows, row_begin = 0, token_begin = 0;

    __device__ void store(int local_row, int token, float value) const {
        const int row     = row_begin + local_row;
        const auto column = std::int64_t(token_begin) + token;
        if (row < channels)
            qkv[column * channels + row] = __float2bfloat16_rn(value);
        else
            z[column * value_rows + row - channels] = __float2bfloat16_rn(value);
    }
};

template <class Publish>
struct ConvOutput {
    GdnConvEpilogue<Publish> conv;
    __nv_bfloat16* z;
    int row_begin = 0;

    template <int Capacity>
    __device__ void store_row(int local_row, const float (&values)[Capacity], int tokens) const {
        const int row    = row_begin + local_row;
        const auto first = std::int64_t(conv.batch_row) * conv.width;
        if (row >= conv.channels) {
            for (int t = 0; t < tokens; ++t)
                z[(first + t) * conv.value_rows + row - conv.channels] =
                    __float2bfloat16_rn(values[t]);
            return;
        }
        const CausalConvOutputColumn<__nv_bfloat16> destination =
            row < conv.query_rows
                ? CausalConvOutputColumn<__nv_bfloat16>{conv.query + row, conv.query_rows}
            : row < conv.query_rows + conv.key_rows
                ? CausalConvOutputColumn<__nv_bfloat16>{conv.key + row - conv.query_rows,
                                                        conv.key_rows}
                : CausalConvOutputColumn<__nv_bfloat16>{
                      conv.value + row - conv.query_rows - conv.key_rows, conv.value_rows};
        const int valid =
            conv.valid_columns ? min(tokens, max(0, conv.valid_columns[conv.batch_row])) : tokens;
        // Inactive rows never read the state selector or publish history/records.
        if (!valid) {
            for (int t = 0; t < tokens; ++t) *destination.at(first + t) = __float2bfloat16_rn(0.0f);
            return;
        }
        const auto source = conv.source_offsets[conv.batch_row] / 2;
        float s0          = __bfloat162float(conv.state_read[source + row]);
        float s1          = __bfloat162float(conv.state_read[source + conv.channels + row]);
        float s2          = __bfloat162float(conv.state_read[source + 2LL * conv.channels + row]);
        const float w0    = __bfloat162float(conv.conv_weight[row]);
        const float w1    = __bfloat162float(conv.conv_weight[conv.channels + row]);
        const float w2    = __bfloat162float(conv.conv_weight[2LL * conv.channels + row]);
        const float w3    = __bfloat162float(conv.conv_weight[3LL * conv.channels + row]);
        for (int t = 0; t < tokens; ++t) {
            if (t >= valid) {
                *destination.at(first + t) = __float2bfloat16_rn(0.0f);
                continue;
            }
            const float p              = values[t];
            *destination.at(first + t) = causal_conv4_silu(w0, w1, w2, w3, s0, s1, s2, p);
            conv.publish.publish(t, conv.batch_row, row, s1, s2, p);
            s0 = s1;
            s1 = s2;
            s2 = p;
        }
    }
};

struct ConvEpilogue {
    template <class Output, int Capacity>
    __device__ void apply_row(const Output& output, int row, int, const float (&values)[Capacity],
                              int tokens) const {
        output.store_row(row, values, tokens);
    }
};

template <int Capacity, class Output, class Epilogue>
void project(const Tensor& x, const WeightOperands& weights, Output output, Epilogue epilogue,
             cudaStream_t stream) {
    switch (weights.format) {
    case Format::Q4Q5:
        launch_packed<Packed::Q4, Capacity>(x, *weights.first, output, epilogue, stream);
        output.row_begin = weights.first->n;
        launch_packed<Packed::Q5, Capacity>(x, *weights.second, output, epilogue, stream);
        return;
    case Format::Q8:
        launch_q8_window<Capacity>(x, *weights.first, output, epilogue, stream);
        return;
    case Format::Fp8:
        launch_packed<Packed::Fp8, Capacity>(x, *weights.first, output, epilogue, stream);
        return;
    case Format::Nvfp4:
        launch_packed<Packed::Nvfp4, Capacity>(x, *weights.first, output, epilogue, stream);
        return;
    }
    throw std::invalid_argument("GDN input r9700: unsupported representation");
}

Tensor columns(const Tensor& x, int first, int count) {
    return Tensor(static_cast<__nv_bfloat16*>(x.data) + std::int64_t(first) * x.ne[0], x.device,
                  x.dtype, {x.ne[0], count});
}

template <class Publish>
void fused(const Tensor& x, const WeightOperands& weights, const Tensor& conv,
           ConstStateFieldView state, const Tensor& valid, GdnConvOutputs out, Batch batch,
           Publish publish, cudaStream_t stream) {
    if (batch.tokens > 16) throw std::logic_error("GDN input r9700: fused window exceeds 16");
    const auto& geometry = weights.geometry;
    ConvOutput<Publish> output{
        {static_cast<const __nv_bfloat16*>(conv.data),
         common::operand::state_field_base<const __nv_bfloat16>(state),
         static_cast<const std::int64_t*>(state.records.byte_offsets.data),
         static_cast<const std::int32_t*>(valid.data), static_cast<__nv_bfloat16*>(out.query.data),
         static_cast<__nv_bfloat16*>(out.key.data), static_cast<__nv_bfloat16*>(out.value.data),
         geometry.channels(), geometry.query_rows, geometry.key_rows, geometry.value_rows, 0,
         batch.tokens, 0, publish},
        static_cast<__nv_bfloat16*>(out.z.data)};
    for (int row = 0; row < batch.rows; ++row) {
        output.conv.batch_row = row;
        project<16>(columns(x, row * batch.tokens, batch.tokens), weights, output, ConvEpilogue{},
                    stream);
    }
}
} // namespace

void launch_projection(const Tensor& x, const WeightOperands& weights, ProjectedOutputs out,
                       ProjectionPlan plan, const Scratch&, cudaStream_t stream) {
    if (plan.compute != Compute::A16)
        throw std::logic_error("GDN input r9700: activation quantization is unavailable");
    SplitOutput output{static_cast<__nv_bfloat16*>(out.qkv.data),
                       static_cast<__nv_bfloat16*>(out.z.data), weights.geometry.channels(),
                       weights.geometry.value_rows};
    for (std::int64_t first = 0; first < x.ne[1]; first += 8) {
        const int count    = static_cast<int>(std::min<std::int64_t>(8, x.ne[1] - first));
        output.token_begin = static_cast<int>(first);
        project<8>(columns(x, output.token_begin, count), weights, output, LinearIdentityEpilogue{},
                   stream);
    }
}

void launch_snapshot(const Tensor& x, const WeightOperands& weights, const Tensor& conv,
                     ConstStateFieldView state, HistoryDestination destination, const Tensor& valid,
                     GdnConvOutputs out, Batch batch, ConvPlan plan, const Scratch& scratch,
                     cudaStream_t stream) {
    if (!plan.materialized()) {
        fused(x, weights, conv, state, valid, out, batch,
              make_snapshot_history_publish(destination, weights.geometry.channels()), stream);
        return;
    }
    const Tensor flat_x = x.view({weights.geometry.input_rows, batch.columns()});
    launch_projection(
        flat_x, weights,
        {scratch.projected, out.z.view({weights.geometry.value_rows, batch.columns()})},
        plan.projection, scratch, stream);
    launch_gdn_projected_conv_snapshot(
        scratch.projected.view({weights.geometry.channels(), batch.tokens, batch.rows}), conv,
        state, valid, destination, out.query, out.key, out.value, stream);
}

void launch_record(const Tensor& x, const WeightOperands& weights, const Tensor& conv,
                   ConstStateFieldView state, const Tensor& valid, GdnConvOutputs out,
                   Tensor& record, Batch batch, ConvPlan plan, const Scratch& scratch,
                   cudaStream_t stream) {
    if (!plan.materialized()) {
        fused(x, weights, conv, state, valid, out, batch,
              RecordColumnPublish{static_cast<__nv_bfloat16*>(record.data),
                                  weights.geometry.channels(), batch.tokens},
              stream);
        return;
    }
    launch_projection(x.view({weights.geometry.input_rows, batch.columns()}), weights,
                      {record.view({weights.geometry.channels(), batch.columns()}),
                       out.z.view({weights.geometry.value_rows, batch.columns()})},
                      plan.projection, scratch, stream);
    launch_gdn_projected_conv_record(record, conv, state, valid, out.query, out.key, out.value,
                                     stream);
}
} // namespace ninfer::ops::detail::gdn_input

namespace ninfer::ops::detail {
std::size_t fp8_gdn_input_partial_capacity_bytes(std::int32_t) {
    throw std::logic_error("GDN input r9700: A8 workspace is unavailable");
}
} // namespace ninfer::ops::detail
