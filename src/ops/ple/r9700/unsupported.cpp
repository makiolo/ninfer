#include "ninfer/ops/ple.h"

#include <stdexcept>

namespace ninfer::ops {

void ngram_embedding_lookup(const Tensor& token_ids, const Tensor& valid_columns,
                            const NgramTokenTailView& before, const NgramHashParameters& hash,
                            std::int32_t eos, std::span<const Nf4EmbeddingShard> shards,
                            Tensor& embedding, CudaExecutionView execution) {
    throw std::invalid_argument("ngram_embedding_lookup: not supported on r9700");
}

void ple_gate(const PleProjectionView& projected, const Tensor& residual,
              const PleGateParameters& parameters, const PleBatchView& batch,
              PleConvRecordWindowView records, Tensor& gated_value, CudaExecutionView execution) {
    throw std::invalid_argument("ple_gate: not supported on r9700");
}

void ple_conv_add(const Tensor& residual, const Tensor& gated_value, const Tensor& kernel,
                  const PleBatchView& batch, const ConstPleConvStateView& before,
                  const PleConvRecordWindowView& current, Tensor& updated_residual,
                  CudaExecutionView execution) {
    throw std::invalid_argument("ple_conv_add: not supported on r9700");
}

void ple_conv_add_norm(const Tensor& residual, const Tensor& gated_value, const Tensor& kernel,
                       const PleBatchView& batch, const ConstPleConvStateView& before,
                       const PleConvRecordWindowView& current, const Tensor& next_hc_norm_weight,
                       float next_hc_epsilon, Tensor& updated_residual, Tensor& normalized,
                       CudaExecutionView execution) {
    throw std::invalid_argument("ple_conv_add_norm: not supported on r9700");
}

void ple_state_extract(const ConstPleConvStateView& before, const PleConvRecordReadView& records,
                       const Tensor& accepted_columns, PleConvStateView after,
                       const Tensor& live_rows, CudaExecutionView execution) {
    throw std::invalid_argument("ple_state_extract: not supported on r9700");
}

} // namespace ninfer::ops
