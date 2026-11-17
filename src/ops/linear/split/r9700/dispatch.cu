#include "ops/linear/split/dispatch.h"
#include "ops/linear/kernels/q4/q4_operands.h"
#include "ops/linear/kernels/q5/q5_operands.h"
#include "ops/linear/kernels/common/r9700/rowsplit.cuh"

#include <algorithm>
#include <ranges>
#include <stdexcept>

namespace ninfer::ops::detail::linear_split {
namespace {
template <class Output>
struct ParentOutput {
    Output output;
    int begin;

    __device__ __forceinline__ void store(int row, int token, float value) const {
        output.store(begin + row, token, value);
    }
};

template <int... Rows>
void launch(const Tensor& x, Weights weights, std::span<const Tensor> outputs, MemoryArena*,
            cudaStream_t stream) {
    const auto first  = q4_linear_operands(x, weights.first);
    const auto second = q5_linear_operands(x, *weights.second);
    // Validate both parents before either can publish an output.
    validate_q4_operands(first);
    validate_q5_operands(second);
    LinearBf16SegmentedOutput<Rows...> output{};
    for (std::size_t i = 0; i < outputs.size(); ++i)
        output.data[i] = static_cast<__nv_bfloat16*>(outputs[i].data);
    const ParentOutput first_out{output, 0};
    const ParentOutput second_out{output, first.rows};
    if (x.ne[1] == 1) {
        launch_rowsplit_a16<4, 1>(first, first_out, LinearIdentityEpilogue{}, stream);
        launch_rowsplit_a16<5, 1>(second, second_out, LinearIdentityEpilogue{}, stream);
    } else {
        launch_rowsplit_a16<4, 8>(first, first_out, LinearIdentityEpilogue{}, stream);
        launch_rowsplit_a16<5, 8>(second, second_out, LinearIdentityEpilogue{}, stream);
    }
}

const SplitShape kAttention{7168, 5120, kSplitRows<6144, 1024, 6144, 1024>,
                            launch<6144, 1024, 6144, 1024>, nullptr, 7168};
const SplitShape kGdn{4096, 5120, kSplitRows<10240, 6144>, launch<10240, 6144>, nullptr, 12288};

const SplitShape& select(ProjectionProfile weights, const auto& rows) {
    if (!weights.second || weights.first.qtype != QType::Q4_G64_FP16 ||
        weights.second->qtype != QType::Q5_G64_FP16 ||
        weights.first.policy != LinearPolicy::A16Only ||
        weights.second->policy != LinearPolicy::A16Only)
        throw std::invalid_argument("linear_split: r9700 requires Q4+Q5 paired A16 weights");
    for (const auto* shape : {&kAttention, &kGdn})
        if (weights.first.n == shape->first_rows && weights.second->n == shape->second_rows &&
            weights.first.k == shape->input_rows && weights.second->k == shape->input_rows &&
            std::ranges::equal(shape->segments, rows))
            return *shape;
    throw std::invalid_argument("linear_split: unsupported r9700 geometry or segmentation");
}
} // namespace

const SplitShape& select_split_shape(ProjectionProfile weights,
                                     std::span<const std::int32_t> rows) {
    return select(weights, rows);
}

const SplitShape& select_split_shape(ProjectionProfile weights, std::span<const Tensor> outputs) {
    return select(weights,
                  outputs | std::views::transform([](const Tensor& t) { return t.ne[0]; }));
}

const SplitBiasShape& select_split_bias_shape(QType, int, int, LinearPolicy,
                                              std::span<const std::int32_t>) {
    throw std::invalid_argument("linear_split bias: not supported on r9700");
}

const SplitBiasShape& select_split_bias_shape(QType, int, int, LinearPolicy,
                                              std::span<const Tensor>) {
    throw std::invalid_argument("linear_split bias: not supported on r9700");
}
} // namespace ninfer::ops::detail::linear_split
