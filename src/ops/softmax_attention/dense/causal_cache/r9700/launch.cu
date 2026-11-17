#include "ops/softmax_attention/dense/causal_cache/bf16/launch.h"
#include "ops/softmax_attention/dense/causal_cache/bf16/plan.h"
#include "ops/softmax_attention/dense/causal_cache/fp8/launch.h"
#include "ops/softmax_attention/dense/causal_cache/fp8/plan.h"
#include "ops/softmax_attention/dense/causal_cache/r9700/plan.h"
#include "ops/softmax_attention/kernels/causal/r9700/attention.cuh"

namespace ninfer::ops::detail {
namespace {

template <bool Fp8, class Cache, class WriteCache>
void attention(const Tensor& q, const Tensor& positions, float scale, Cache cache,
               const Tensor* valid, const Tensor* rows, CausalAttentionExecutionEnvelope envelope,
               MemoryArena& workspace, Tensor& out, CudaExecutionView execution,
               WriteCache write_cache) {
    const auto p = make_causal_operands(q, positions, out, scale, envelope.max_visible_keys);
    const int splits = r9700::causal_splits(p.width, p.batch);
    auto scope = workspace.scope();
    CausalPartialStorage storage;
    if (splits != 1)
        storage = allocate_causal_partials(workspace, p.query_heads, p.width, splits, p.batch);
    const auto view = [&] {
        if constexpr (Fp8)
            return make_quantized_causal_cache_view<Fp8KvReadView>(cache, valid, rows);
        else
            return bf16_kv_cache_view<false>(cache, valid, rows);
    }();
    // Acquire all scratch before the first persistent mutation. The writer and
    // attention then execute in stream order with caller-owned backing.
    write_cache();
    const dim3 grid(p.width, p.query_heads, p.batch * splits);
    r9700::causal_partition<Fp8><<<grid, 128, 0, execution.stream()>>>(p, view, splits, storage.view());
    CUDA_CHECK(cudaGetLastError());
    if (splits != 1) {
        r9700::causal_merge<<<dim3(p.width, p.query_heads, p.batch), 256, 0, execution.stream()>>>(
            p, splits, storage.view());
        CUDA_CHECK(cudaGetLastError());
    }
}

template <bool Fp8>
void append(const Tensor& q, const Tensor& k, const Tensor& v, const Tensor& positions,
            const Tensor& valid, const Tensor& rows, float scale, PagedKVBatchLayerView cache,
            CausalAttentionExecutionEnvelope envelope, MemoryArena& workspace, Tensor& out,
            CudaExecutionView execution) {
    attention<Fp8>(q, positions, scale, cache, &valid, &rows, envelope, workspace, out, execution,
                    [&] {
                        kv::launch_paged_write(kv::PagedWriteInstance::BatchRows256, k, v,
                                               positions, valid, rows, cache, execution.stream());
                    });
}

} // namespace

std::size_t bf16_kv_workspace_bytes(int heads, int batch, int first, int last,
                                   CausalAttentionExecutionEnvelope, int) {
    return r9700::causal_workspace_bytes(heads, batch, first, last);
}

std::size_t fp8_kv_workspace_bytes(int heads, int batch, int first, int last,
                                  CausalAttentionExecutionEnvelope, int) {
    return r9700::causal_workspace_bytes(heads, batch, first, last);
}

void bf16_kv_append_attention(const Tensor& q, const Tensor& k, const Tensor& v,
                              const Tensor& positions, const Tensor& valid, const Tensor& rows,
                              float scale, PagedKVBatchLayerView cache,
                              CausalAttentionExecutionEnvelope envelope, MemoryArena& workspace,
                              Tensor& out, CudaExecutionView execution) {
    append<false>(q, k, v, positions, valid, rows, scale, cache, envelope, workspace, out, execution);
}

void fp8_kv_append_attention(const Tensor& q, const Tensor& k, const Tensor& v,
                             const Tensor& positions, const Tensor& valid, const Tensor& rows,
                             float scale, PagedKVBatchLayerView cache,
                             CausalAttentionExecutionEnvelope envelope, MemoryArena& workspace,
                             Tensor& out, CudaExecutionView execution) {
    append<true>(q, k, v, positions, valid, rows, scale, cache, envelope, workspace, out, execution);
}

void bf16_kv_cached_attention(const Tensor& q, const Tensor& positions, float scale,
                              const ConstPagedKVLayerView& cache,
                              CausalAttentionExecutionEnvelope envelope, MemoryArena& workspace,
                              Tensor& out, CudaExecutionView execution) {
    attention<false>(q, positions, scale, single_row_paged_kv_batch_view(cache), nullptr, nullptr,
                      envelope, workspace, out, execution, [] {});
}

void fp8_kv_cached_attention(const Tensor& q, const Tensor& positions, float scale,
                             const ConstPagedKVLayerView& cache,
                             CausalAttentionExecutionEnvelope envelope, MemoryArena& workspace,
                             Tensor& out, CudaExecutionView execution) {
    attention<true>(q, positions, scale, single_row_paged_kv_batch_view(cache), nullptr, nullptr,
                     envelope, workspace, out, execution, [] {});
}

} // namespace ninfer::ops::detail
