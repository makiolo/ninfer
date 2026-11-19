#include "ops/softmax_attention/kpool_mla/prepare/arguments.h"

#include <stdexcept>

namespace ninfer::ops::detail::kpool {
namespace {
[[noreturn]] void unsupported() {
    throw std::invalid_argument("kpool_mla_prepare: not supported on r9700");
}
} // namespace

PrepareGeometry prepare_geometry(const KpoolMlaPrepareProfile&, KpoolMlaStorage,
                                 KvCacheVectorFormat, int, int, int, int) {
    unsupported();
}

PrepareProjectionPlan prepare_projection_plan(const PrepareGeometry&, int) { unsupported(); }

int prepare_window_columns() { unsupported(); }

void launch_prepare(const PrepareArguments&, const PrepareGeometry&, int, cudaStream_t) {
    unsupported();
}
} // namespace ninfer::ops::detail::kpool

#include "ops/softmax_attention/kpool_mla/indexer/plan.h"

#include <stdexcept>

namespace ninfer::ops::detail::kpool {
void indexer_geometry(const KpoolIndexerProfile&, KvCacheVectorFormat, KpoolExecutionEnvelope, int,
                      int, int, int) {
    throw std::invalid_argument("kpool_indexer: not supported on r9700");
}

void launch_indexer(IndexerArguments, int, cudaStream_t) {
    throw std::invalid_argument("kpool_indexer: not supported on r9700");
}

void launch_fp8_indexer(Fp8IndexerArguments, int, cudaStream_t) {
    throw std::invalid_argument("kpool_indexer FP8: not supported on r9700");
}

int indexer_group_size(int, int) {
    throw std::invalid_argument("kpool_indexer: not supported on r9700");
}

Fp8IndexerWindows fp8_indexer_windows(int, int, int) {
    throw std::invalid_argument("kpool_indexer FP8: not supported on r9700");
}

IndexerWindows indexer_windows(int, int, int, int) {
    throw std::invalid_argument("kpool_indexer: not supported on r9700");
}
} // namespace ninfer::ops::detail::kpool

#include "ops/softmax_attention/kpool_mla/sparse_mla/plan.h"

#include <stdexcept>

namespace ninfer::ops::detail::kpool {
void sparse_mla_geometry(const KpoolSparseMlaProfile&, KpoolMlaStorage, KpoolExecutionEnvelope, int,
                         int, int, int) {
    throw std::invalid_argument("kpool_sparse_mla: not supported on r9700");
}

std::size_t sparse_mla_capacity(const KpoolSparseMlaProfile&, KpoolMlaStorage, int, int, int,
                                KpoolExecutionEnvelope, int) {
    throw std::invalid_argument("kpool_sparse_mla: not supported on r9700");
}

SparseMlaPlan sparse_mla_plan(const KpoolSparseMlaProfile&, KpoolMlaStorage, int, int,
                              KpoolExecutionEnvelope, int) {
    throw std::invalid_argument("kpool_sparse_mla: not supported on r9700");
}

void launch_sparse_mla(SparseMlaArguments, MemorySpan, const KpoolSparseMlaProfile&,
                       KpoolMlaStorage, KpoolExecutionEnvelope, int, cudaStream_t) {
    throw std::invalid_argument("kpool_sparse_mla: not supported on r9700");
}
} // namespace ninfer::ops::detail::kpool

namespace ninfer::ops {
void kpool_tail_extract(ConstKpoolTailStateView before, const KpoolRawRecordReadView& records,
                        const Tensor& accepted_columns, KpoolTailStateView after,
                        CudaExecutionView execution) {
    throw std::invalid_argument("kpool_tail_extract: not supported on r9700");
}
} // namespace ninfer::ops
