#include "ninfer/ops/qsa.h"

#include <stdexcept>

namespace ninfer::ops {

std::size_t qsa_indexer_workspace_capacity_bytes(std::int32_t min_tokens, std::int32_t max_tokens,
                                                 std::int32_t batch_size,
                                                 QsaIndexerExecutionEnvelope envelope) {
    throw std::invalid_argument("qsa_indexer_workspace_capacity_bytes: not supported on r9700");
}

void qsa_indexer(const Tensor& index_query, const ConstQsaIndexCacheView& cache,
                 const QsaBatchView& batch, QsaIndexerExecutionEnvelope envelope,
                 MemoryArena& workspace, QsaBlockSelection& out, CudaExecutionView execution) {
    throw std::invalid_argument("qsa_indexer: not supported on r9700");
}

std::size_t qsa_sparse_gqa_workspace_capacity_bytes(
    KvCacheStorage storage, std::int32_t query_heads, std::int32_t kv_heads,
    std::int32_t min_tokens, std::int32_t max_tokens, std::int32_t batch_size,
    QsaSparseGqaExecutionEnvelope envelope, std::int32_t multiprocessor_count) {
    throw std::invalid_argument("qsa_sparse_gqa_workspace_capacity_bytes: not supported on r9700");
}

void qsa_sparse_gqa(const Tensor& query, const Tensor& output_gate,
                    const ConstPagedKVBatchLayerView& cache, const QsaBatchView& batch,
                    const QsaBlockSelection& selected, QsaSparseGqaExecutionEnvelope envelope,
                    MemoryArena& workspace, Tensor& out, CudaExecutionView execution) {
    throw std::invalid_argument("qsa_sparse_gqa: not supported on r9700");
}

void qsa_prepare(const QsaProjectionView& projected, const QsaPrepareParameters& parameters,
                 const QsaBatchView& batch, const Tensor& rope_positions,
                 const ConstQsaTailStateView& before, QsaCacheView cache,
                 QsaRawRecordWriteView records, Tensor& query, Tensor& index_query,
                 CudaExecutionView execution) {
    throw std::invalid_argument("qsa_prepare: not supported on r9700");
}

void qsa_prepare_kv(const QsaKvProjectionView& projected, const QsaKvPrepareParameters& parameters,
                    const QsaBatchView& batch, const Tensor& rope_positions,
                    const ConstQsaTailStateView& before, QsaCacheView cache,
                    QsaRawRecordWriteView records, CudaExecutionView execution) {
    throw std::invalid_argument("qsa_prepare_kv: not supported on r9700");
}

void qsa_tail_extract(const ConstQsaTailStateView& before, const QsaRawRecordReadView& records,
                      const Tensor& accepted_columns, QsaTailStateView after,
                      const Tensor& live_rows, CudaExecutionView execution) {
    throw std::invalid_argument("qsa_tail_extract: not supported on r9700");
}

} // namespace ninfer::ops
