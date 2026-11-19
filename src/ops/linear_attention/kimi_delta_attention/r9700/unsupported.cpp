#include "ninfer/ops/kimi_delta_attention.h"

#include <stdexcept>

namespace ninfer::ops {

std::size_t kimi_delta_attention_workspace_capacity_bytes(std::int32_t qk_heads,
                                                          std::int32_t value_heads,
                                                          std::int32_t min_tokens,
                                                          std::int32_t max_tokens) {
    throw std::invalid_argument(
        "kimi_delta_attention_workspace_capacity_bytes: not supported on r9700");
}

void kimi_delta_attention(const Tensor& q, const Tensor& k, const Tensor& v, const Tensor& g,
                          const Tensor& beta, const Tensor& A_log, const Tensor& dt_bias,
                          float lower_bound, float scale, MemoryArena& workspace, Tensor& state,
                          Tensor& out, CudaExecutionView execution) {
    throw std::invalid_argument("kimi_delta_attention: not supported on r9700");
}

void kimi_delta_attention(const Tensor& q, const Tensor& k, const Tensor& v, const Tensor& g,
                          const Tensor& beta, const Tensor& A_log, const Tensor& dt_bias,
                          float lower_bound, float scale, MemoryArena& workspace,
                          const Tensor& state_in, Tensor& state_out, Tensor& out,
                          CudaExecutionView execution) {
    throw std::invalid_argument("kimi_delta_attention: not supported on r9700");
}

void kimi_delta_attention_batch_update(const Tensor& q, const Tensor& k, const Tensor& v,
                                       const Tensor& g, const Tensor& beta, const Tensor& A_log,
                                       const Tensor& dt_bias, float lower_bound, float scale,
                                       ConstStateFieldView source, StateFieldView destination,
                                       const Tensor& valid_columns, Tensor& out,
                                       CudaExecutionView execution) {
    throw std::invalid_argument("kimi_delta_attention_batch_update: not supported on r9700");
}

void kimi_delta_attention_replay_record(const Tensor& q, const Tensor& k, const Tensor& v,
                                        const Tensor& g, const Tensor& beta, const Tensor& A_log,
                                        const Tensor& dt_bias, float lower_bound, float scale,
                                        ConstStateFieldView initial_state,
                                        const Tensor& valid_columns, KdaRecurrenceRecords records,
                                        Tensor& out, CudaExecutionView execution) {
    throw std::invalid_argument("kimi_delta_attention_replay_record: not supported on r9700");
}

} // namespace ninfer::ops
