#include "ops/softmax_attention/dense/causal_cache/int8/launch.h"
#include "ops/softmax_attention/dense/causal_cache/int8/plan.h"
#include "ops/softmax_attention/dense/causal_cache/nvfp4/launch.h"
#include "ops/softmax_attention/dense/causal_cache/nvfp4/plan.h"
#include "ops/softmax_attention/dense/causal_cache/k8v4/launch.h"
#include "ops/softmax_attention/dense/causal_cache/k8v4/plan.h"

#include <stdexcept>
#include <string>

namespace ninfer::ops::detail {
namespace {
[[noreturn]] void unsupported(const char* format) {
    throw std::invalid_argument(std::string("causal_softmax_attention: ") + format +
                                " cache is not supported on r9700 (supported: BF16, FP8)");
}
} // namespace

// Keep support decisions behind the same per-format bindings as CUDA. Both the
// capacity query and execution reject before writing any persistent cache data.
#define NINFER_UNSUPPORTED_CAUSAL(prefix, format)                                               \
    std::size_t prefix##_kv_workspace_bytes(int, int, int, int,                                 \
                                             CausalAttentionExecutionEnvelope, int) {          \
        unsupported(format);                                                                  \
    }                                                                                         \
    void prefix##_kv_append_attention(const Tensor&, const Tensor&, const Tensor&,              \
                                       const Tensor&, const Tensor&, const Tensor&, float,      \
                                       PagedKVBatchLayerView, CausalAttentionExecutionEnvelope, \
                                       MemoryArena&, Tensor&, CudaExecutionView) {              \
        unsupported(format);                                                                  \
    }                                                                                         \
    void prefix##_kv_cached_attention(const Tensor&, const Tensor&, float,                      \
                                       const ConstPagedKVLayerView&,                           \
                                       CausalAttentionExecutionEnvelope, MemoryArena&,         \
                                       Tensor&, CudaExecutionView) {                            \
        unsupported(format);                                                                  \
    }

NINFER_UNSUPPORTED_CAUSAL(int8, "INT8")
NINFER_UNSUPPORTED_CAUSAL(nvfp4, "NVFP4")
NINFER_UNSUPPORTED_CAUSAL(k8v4, "K8V4")
#undef NINFER_UNSUPPORTED_CAUSAL

} // namespace ninfer::ops::detail
