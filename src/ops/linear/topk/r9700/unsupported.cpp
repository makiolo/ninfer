#include "ninfer/ops/linear_topk.h"

#include <stdexcept>

namespace ninfer::ops {

std::size_t linear_topk_workspace_capacity_bytes(QType qtype, std::int32_t head_rows,
                                                 std::int32_t input_rows, std::int32_t min_columns,
                                                 std::int32_t max_columns) {
    throw std::invalid_argument("linear_topk_workspace_capacity_bytes: not supported on r9700");
}

void linear_topk(const Tensor& hidden, const Weight& head, std::int32_t valid_rows,
                 Tensor& candidate_ids, Tensor& candidate_scores, MemoryArena& workspace,
                 CudaExecutionView execution) {
    throw std::invalid_argument("linear_topk: not supported on r9700");
}

void linear_topk(const Tensor& hidden, const Weight& head, const Tensor& row_to_global_ids,
                 Tensor& candidate_ids, Tensor& candidate_scores, MemoryArena& workspace,
                 CudaExecutionView execution) {
    throw std::invalid_argument("linear_topk: not supported on r9700");
}

std::size_t linear_topk_workspace_capacity_bytes(QType qtype, std::int32_t head_rows,
                                                 std::int32_t input_rows, LinearPolicy policy,
                                                 std::int32_t min_columns,
                                                 std::int32_t max_columns) {
    throw std::invalid_argument("linear_topk_workspace_capacity_bytes: not supported on r9700");
}

void linear_topk(const Tensor& hidden, const Weight& head, std::int32_t token_begin,
                 std::int32_t token_domain, LinearPolicy policy, Tensor& keys,
                 MemoryArena& workspace, CudaExecutionView execution) {
    throw std::invalid_argument("linear_topk: not supported on r9700");
}

} // namespace ninfer::ops
