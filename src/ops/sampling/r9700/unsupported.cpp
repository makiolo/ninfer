#include "ninfer/ops/speculative_accept.h"
#include "ninfer/ops/candidate_selector.h"
#include "ninfer/ops/vocabulary.h"

#include <stdexcept>

namespace ninfer::ops {

std::size_t speculative_accept_sparse_drafts_workspace_capacity_bytes(
    std::int32_t token_domain, SpeculativeAcceptExecutionEnvelope envelope, std::int32_t min_drafts,
    std::int32_t max_drafts, std::int32_t min_batch, std::int32_t max_batch) {
    throw std::invalid_argument(
        "speculative_accept_sparse_drafts_workspace_capacity_bytes: not supported on r9700");
}

void speculative_accept_sparse_drafts(const Tensor& target_tokens, const Tensor& logits,
                                      const Tensor& drafts, const Tensor& candidate_ids,
                                      const Tensor& proposal_q, const Tensor& current_extents,
                                      const Tensor& base_positions, Tensor& licensed_tokens,
                                      Tensor& licensed_counts, std::int32_t token_domain,
                                      const SamplingConfig* configs,
                                      SpeculativeAcceptExecutionEnvelope envelope,
                                      const Tensor& live_rows, MemoryArena& workspace,
                                      CudaExecutionView execution) {
    throw std::invalid_argument("speculative_accept_sparse_drafts: not supported on r9700");
}

std::size_t candidate_selector_path_workspace_capacity_bytes(CandidateSelectorInput input,
                                                             std::int32_t min_steps,
                                                             std::int32_t max_steps,
                                                             std::int32_t min_batch,
                                                             std::int32_t max_batch) {
    throw std::invalid_argument(
        "candidate_selector_path_workspace_capacity_bytes: not supported on r9700");
}

void candidate_selector_path(const Tensor& candidate_ids, const Tensor& unary_scores,
                             const Tensor& projected_hidden, const Tensor& anchors,
                             const Tensor& predecessor_codebook, const Tensor& successor_codebook,
                             std::int32_t token_domain, const Tensor& base_positions,
                             const SamplingConfig* configs, Tensor& drafts, Tensor& proposal_q,
                             const Tensor& live_rows, MemoryArena& workspace,
                             CudaExecutionView execution) {
    throw std::invalid_argument("candidate_selector_path: not supported on r9700");
}

void candidate_selector_path(const Tensor& keys, const Tensor& projected_hidden,
                             const Tensor& anchors, const Tensor& predecessor_codebook,
                             const Tensor& successor_codebook, std::int32_t token_domain,
                             const Tensor& base_positions, const SamplingConfig* configs,
                             Tensor& candidate_ids, Tensor& drafts, Tensor& proposal_q,
                             const Tensor& live_rows, MemoryArena& workspace,
                             CudaExecutionView execution) {
    throw std::invalid_argument("candidate_selector_path: not supported on r9700");
}

std::size_t vocabulary_candidates_workspace_capacity_bytes(std::int32_t physical_rows,
                                                           std::int64_t max_columns,
                                                           std::int32_t candidates) {
    throw std::invalid_argument(
        "vocabulary_candidates_workspace_capacity_bytes: not supported on r9700");
}

void vocabulary_candidates(const Tensor& logits, std::int32_t token_begin,
                           std::int32_t token_domain, const SamplingConfig* configs,
                           const Tensor& valid_columns, const Tensor& drafts, Tensor& keys,
                           MemoryArena& workspace, CudaExecutionView execution) {
    throw std::invalid_argument("vocabulary_candidates: not supported on r9700");
}

void sample_vocabulary_candidates(const Tensor& keys, const SamplingConfig* configs,
                                  const Tensor& logical_positions, std::int32_t purpose,
                                  Tensor& output, const Tensor& live_rows,
                                  CudaExecutionView execution) {
    throw std::invalid_argument("sample_vocabulary_candidates: not supported on r9700");
}

void accept_vocabulary_candidates(const Tensor& keys, const SamplingConfig* configs,
                                  const Tensor& drafts, const Tensor& current_extents,
                                  const Tensor& base_positions, Tensor& licensed_tokens,
                                  Tensor& licensed_counts, const Tensor& live_rows,
                                  CudaExecutionView execution) {
    throw std::invalid_argument("accept_vocabulary_candidates: not supported on r9700");
}

void accept_vocabulary_candidates_sparse(const Tensor& keys, const SamplingConfig* configs,
                                         const Tensor& drafts, const Tensor& candidate_ids,
                                         const Tensor& proposal_q, const Tensor& current_extents,
                                         const Tensor& base_positions, Tensor& licensed_tokens,
                                         Tensor& licensed_counts, const Tensor& live_rows,
                                         CudaExecutionView execution) {
    throw std::invalid_argument("accept_vocabulary_candidates_sparse: not supported on r9700");
}

} // namespace ninfer::ops
