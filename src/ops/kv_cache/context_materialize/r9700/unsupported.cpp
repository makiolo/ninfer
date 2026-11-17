#include "ninfer/ops/context_kv_materialize.h"

#include <stdexcept>

namespace ninfer::ops {

void context_kv_materialize(
    const Tensor& context, const Tensor& positions, const Tensor& counts,
    const std::array<ContextKVMaterializeLayerView, kContextKVMaterializeLayers>& layers,
    const ContextKVMaterializeProfile& profile, ContextKVMaterializeExecutionEnvelope envelope,
    MemoryArena& workspace, CudaExecutionView execution) {
    throw std::invalid_argument("context_kv_materialize: not supported on r9700");
}

std::size_t
context_kv_materialize_workspace_capacity_bytes(const ContextKVMaterializeProfile& profile,
                                                std::int32_t batch_size, std::int32_t min_width,
                                                std::int32_t max_width) {
    throw std::invalid_argument(
        "context_kv_materialize_workspace_capacity_bytes: not supported on r9700");
}

} // namespace ninfer::ops
