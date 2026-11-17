#pragma once

// CUDA spelling is the source contract; the HIP backend owns its translation.
// Include this instead of CUDA SDK headers when compiling shared source as HIP.
#if defined(__HIP_PLATFORM_AMD__) || defined(__HIP__)

#include <hip/hip_runtime.h>
#include <hip/hip_bf16.h>
#include <hip/hip_fp8.h>
#include <hip/hip_math_constants.h>

#define CUDART_INF_F HIP_INF_F

#include "core/platform.h"

#if NINFER_TARGET_R9700
// HIP has no CUDA grid-constant attribute; const kernel arguments remain passed by value.
#define __grid_constant__
#define cudaErrorNoDevice hipErrorNoDevice
#define cudaErrorInsufficientDriver hipErrorInsufficientDriver
#define cudaMemcpy2D hipMemcpy2D
#define cudaMemcpyDefault hipMemcpyDefault
#define cudaStreamCaptureModeGlobal hipStreamCaptureModeGlobal

// The legacy CUDA graph overload is still used by Op contract tests. HIP has
// the matching diagnostics signature; keep it alongside the flags overload.
inline hipError_t cudaGraphInstantiate(hipGraphExec_t* exec, hipGraph_t graph,
                                      hipGraphNode_t* error_node, char* log, size_t log_size) {
    return hipGraphInstantiate(exec, graph, error_node, log, log_size);
}

#if defined(__HIP_DEVICE_COMPILE__)
// Clang exposes ballot_w32 only with the wavefrontsize32 target feature.
// Query that feature without the deprecated __AMDGCN_WAVEFRONT_SIZE macros.
static_assert(__has_builtin(__builtin_amdgcn_ballot_w32),
              "NINFER_TARGET_R9700 requires wave32 device compilation (32-lane wavefronts)");
#endif

// The SDK declares device intrinsics only in HIP language mode. Ordinary C++
// translation units use the runtime/type mappings below, not these overloads.
#if defined(__HIP__)
// CUDA uses 32-bit masks; HIP 7.2's sync intrinsics require an explicit
// 64-bit mask even in wave32. Keep that translation here, retaining the SDK's
// participation checks, width semantics and wavefront memory fences.
template <class T>
__device__ __forceinline__ T __shfl_sync(unsigned mask, T value, int lane, int width = warpSize) {
    return __shfl_sync<unsigned long long, T>(mask, value, lane, width);
}
template <class T>
__device__ __forceinline__ T __shfl_up_sync(unsigned mask, T value, unsigned delta, int width = warpSize) {
    return __shfl_up_sync<unsigned long long, T>(mask, value, delta, width);
}
template <class T>
__device__ __forceinline__ T __shfl_down_sync(unsigned mask, T value, unsigned delta, int width = warpSize) {
    return __shfl_down_sync<unsigned long long, T>(mask, value, delta, width);
}
template <class T>
__device__ __forceinline__ T __shfl_xor_sync(unsigned mask, T value, int lane_mask, int width = warpSize) {
    return __shfl_xor_sync<unsigned long long, T>(mask, value, lane_mask, width);
}
__device__ __forceinline__ unsigned __ballot_sync(unsigned mask, int predicate) {
    return static_cast<unsigned>(__ballot_sync<unsigned long long>(mask, predicate));
}
__device__ __forceinline__ void __syncwarp(unsigned mask) {
    __syncwarp<unsigned long long>(mask);
}
__device__ __forceinline__ int __all_sync(unsigned mask, int predicate) {
    return __all_sync<unsigned long long>(mask, predicate);
}
__device__ __forceinline__ int __any_sync(unsigned mask, int predicate) {
    return __any_sync<unsigned long long>(mask, predicate);
}
template <class T>
__device__ __forceinline__ unsigned __match_any_sync(unsigned mask, T value) {
    return static_cast<unsigned>(__match_any_sync<unsigned long long, T>(mask, value));
}

#define NINFER_HIP_REDUCE_MASK_OVERLOAD(name, type)                         \
    __device__ __forceinline__ type name(unsigned mask, type value) {      \
        return name<unsigned long long>(mask, value);                     \
    }
NINFER_HIP_REDUCE_MASK_OVERLOAD(__reduce_add_sync, int)
NINFER_HIP_REDUCE_MASK_OVERLOAD(__reduce_add_sync, unsigned)
NINFER_HIP_REDUCE_MASK_OVERLOAD(__reduce_min_sync, int)
NINFER_HIP_REDUCE_MASK_OVERLOAD(__reduce_min_sync, unsigned)
NINFER_HIP_REDUCE_MASK_OVERLOAD(__reduce_max_sync, int)
NINFER_HIP_REDUCE_MASK_OVERLOAD(__reduce_max_sync, unsigned)
#undef NINFER_HIP_REDUCE_MASK_OVERLOAD

namespace ninfer::hip_compat {
// HIP 7.2's sparse-mask bit reductions use logical operators in their fallback
// tree. Gather participating lanes with SDK shuffles and apply bitwise operators.
template <char Op>
__device__ __forceinline__ unsigned reduce_bits(unsigned mask, unsigned value) {
    unsigned result = Op == '&' ? ~0u : 0u;
    for (unsigned remaining = mask; remaining; remaining &= remaining - 1) {
        const unsigned other = __shfl_sync(mask, value, __ffs(remaining) - 1);
        if constexpr (Op == '&') result &= other;
        if constexpr (Op == '|') result |= other;
        if constexpr (Op == '^') result ^= other;
    }
    return result;
}
} // namespace ninfer::hip_compat
__device__ __forceinline__ unsigned __reduce_and_sync(unsigned mask, unsigned value) {
    return ninfer::hip_compat::reduce_bits<'&'>(mask, value);
}
__device__ __forceinline__ unsigned __reduce_or_sync(unsigned mask, unsigned value) {
    return ninfer::hip_compat::reduce_bits<'|'>(mask, value);
}
__device__ __forceinline__ unsigned __reduce_xor_sync(unsigned mask, unsigned value) {
    return ninfer::hip_compat::reduce_bits<'^'>(mask, value);
}
#endif // __HIP__
#endif // NINFER_TARGET_R9700

// HIP 7.2 has separate OCP and FNUZ types. gfx12 uses OCP E4M3FN:
// signed zero, subnormals, max finite 448, and NaN at +/-0x7f.
using __nv_bfloat16 = __hip_bfloat16;
using __nv_bfloat162 = __hip_bfloat162;
using __nv_bfloat16_raw = __hip_bfloat16_raw;
using __nv_bfloat162_raw = __hip_bfloat162_raw;
using __nv_fp8_storage_t = __hip_fp8_storage_t;
using __nv_fp8x2_storage_t = __hip_fp8x2_storage_t;
using __nv_saturation_t = __hip_saturation_t;
using __nv_fp8_interpretation_t = __hip_fp8_interpretation_t;
inline constexpr auto __NV_SATFINITE = __HIP_SATFINITE;
inline constexpr auto __NV_E4M3 = __HIP_E4M3;

// HIP 7.2's scalar SATFINITE propagates infinity to NaN; its device x2
// conversion also passes the original, unclipped lanes to the instruction.
// Clip infinities as well as finite overflow before the scalar SDK conversion.
// Comparisons leave NaNs and signed zero intact. All rounding stays in the SDK.
__host__ __device__ inline __nv_fp8_storage_t __nv_cvt_float_to_fp8(
    float value, __nv_saturation_t saturation, __nv_fp8_interpretation_t interpretation) {
    if (saturation == __NV_SATFINITE && interpretation == __NV_E4M3) {
        if (value > 448.0f) value = 448.0f;
        if (value < -448.0f) value = -448.0f;
    }
    return __hip_cvt_float_to_fp8(value, saturation, interpretation);
}
__host__ __device__ inline __nv_fp8x2_storage_t __nv_cvt_float2_to_fp8x2(
    float2 value, __nv_saturation_t saturation, __nv_fp8_interpretation_t interpretation) {
    const auto lo = __nv_cvt_float_to_fp8(value.x, saturation, interpretation);
    const auto hi = __nv_cvt_float_to_fp8(value.y, saturation, interpretation);
    return static_cast<__nv_fp8x2_storage_t>(lo | (static_cast<unsigned int>(hi) << 8));
}

// Storage and decode match OCP in the SDK. These small value types ensure their
// float constructors also use the corrected SATFINITE encoder above.
struct __nv_fp8_e4m3 {
    __nv_fp8_storage_t __x;
    __host__ __device__ __nv_fp8_e4m3() = default;
    __host__ __device__ explicit __nv_fp8_e4m3(float value)
        : __x(__nv_cvt_float_to_fp8(value, __NV_SATFINITE, __NV_E4M3)) {}
    __host__ __device__ explicit operator float() const {
        __hip_fp8_e4m3 value;
        value.__x = __x;
        return static_cast<float>(value);
    }
    __host__ __device__ explicit operator __half() const {
        __hip_fp8_e4m3 value;
        value.__x = __x;
        return static_cast<__half>(value);
    }
};
struct __nv_fp8x2_e4m3 {
    __nv_fp8x2_storage_t __x;
    __host__ __device__ __nv_fp8x2_e4m3() = default;
    __host__ __device__ explicit __nv_fp8x2_e4m3(float2 value)
        : __x(__nv_cvt_float2_to_fp8x2(value, __NV_SATFINITE, __NV_E4M3)) {}
    __host__ __device__ explicit operator float2() const {
        __hip_fp8x2_e4m3 value;
        value.__x = __x;
        return static_cast<float2>(value);
    }
    __host__ __device__ explicit operator __half2() const {
        __hip_fp8x2_e4m3 value;
        value.__x = __x;
        return static_cast<__half2>(value);
    }
};

// HIP's float -> __bf16 conversion is RNE, including gradual underflow and
// NaN preservation. It lacks these two CUDA spellings; reuse the SDK conversion.
__host__ __device__ inline __nv_bfloat16 __float2bfloat16_rn(float value) {
    return __float2bfloat16(value);
}
__host__ __device__ inline __nv_bfloat162 __floats2bfloat162_rn(float x, float y) {
    return __halves2bfloat162(__float2bfloat16_rn(x), __float2bfloat16_rn(y));
}

static_assert(sizeof(__nv_bfloat16) == 2 && sizeof(__nv_bfloat162) == 4);
static_assert(sizeof(__nv_fp8_e4m3) == 1 && sizeof(__nv_fp8x2_e4m3) == 2);

// Runtime names with matching signatures and contracts in HIP 7.2.
#define cudaError_t hipError_t
#define cudaSuccess hipSuccess
#define cudaErrorMemoryAllocation hipErrorOutOfMemory
#define cudaErrorNotReady hipErrorNotReady
#define cudaGetErrorName hipGetErrorName
#define cudaGetErrorString hipGetErrorString
#define cudaGetLastError hipGetLastError
#define cudaPeekAtLastError hipPeekAtLastError
#define cudaDeviceProp hipDeviceProp_t
#define cudaUUID_t hipUUID
#define cudaGetDevice hipGetDevice
#define cudaSetDevice hipSetDevice
#define cudaGetDeviceCount hipGetDeviceCount
#define cudaGetDeviceProperties hipGetDeviceProperties
#define cudaDeviceSynchronize hipDeviceSynchronize
#define cudaGetDeviceFlags hipGetDeviceFlags
#define cudaSetDeviceFlags hipSetDeviceFlags
#define cudaDeviceScheduleAuto hipDeviceScheduleAuto
#define cudaDeviceScheduleBlockingSync hipDeviceScheduleBlockingSync
#define cudaDeviceScheduleMask hipDeviceScheduleMask
#define cudaDeviceScheduleSpin hipDeviceScheduleSpin
#define cudaDeviceScheduleYield hipDeviceScheduleYield
#define cudaDriverGetVersion hipDriverGetVersion
#define cudaRuntimeGetVersion hipRuntimeGetVersion
#define cudaProfilerStart hipProfilerStart
#define cudaProfilerStop hipProfilerStop
#define cudaStream_t hipStream_t
#define cudaStreamCreate hipStreamCreate
#define cudaStreamCreateWithFlags hipStreamCreateWithFlags
#define cudaStreamDestroy hipStreamDestroy
#define cudaStreamSynchronize hipStreamSynchronize
#define cudaStreamQuery hipStreamQuery
#define cudaStreamWaitEvent hipStreamWaitEvent
#define cudaStreamGetDevice hipStreamGetDevice
#define cudaStreamNonBlocking hipStreamNonBlocking
#define cudaStreamBeginCapture hipStreamBeginCapture
#define cudaStreamEndCapture hipStreamEndCapture
#define cudaStreamIsCapturing hipStreamIsCapturing
#define cudaStreamCaptureModeThreadLocal hipStreamCaptureModeThreadLocal
#define cudaStreamCaptureStatus hipStreamCaptureStatus
#define cudaStreamCaptureStatusActive hipStreamCaptureStatusActive
#define cudaStreamCaptureStatusNone hipStreamCaptureStatusNone
#define cudaStreamSetCaptureDependencies hipStreamSetCaptureDependencies
#define cudaEvent_t hipEvent_t
#define cudaEventCreate hipEventCreate
#define cudaEventCreateWithFlags hipEventCreateWithFlags
#define cudaEventDestroy hipEventDestroy
#define cudaEventDisableTiming hipEventDisableTiming
#define cudaEventElapsedTime hipEventElapsedTime
#define cudaEventQuery hipEventQuery
#define cudaEventRecord hipEventRecord
#define cudaEventRecordWithFlags hipEventRecordWithFlags
#define cudaEventRecordExternal hipEventRecordExternal
#define cudaEventWaitExternal hipEventWaitExternal
#define cudaEventSynchronize hipEventSynchronize
#define cudaMalloc hipMalloc
#define cudaMallocHost hipHostMalloc
#define cudaFree hipFree
#define cudaFreeHost hipHostFree
#define cudaHostGetDevicePointer hipHostGetDevicePointer
#define cudaHostRegister hipHostRegister
#define cudaHostRegisterDefault hipHostRegisterDefault
#define cudaHostUnregister hipHostUnregister
#define cudaMemGetInfo hipMemGetInfo
#define cudaMemset hipMemset
#define cudaMemsetAsync hipMemsetAsync
#define cudaMemcpy hipMemcpy
#define cudaMemcpyAsync hipMemcpyAsync
#define cudaMemcpy2DAsync hipMemcpy2DAsync
#define cudaMemcpyKind hipMemcpyKind
#define cudaMemcpyHostToDevice hipMemcpyHostToDevice
#define cudaMemcpyDeviceToHost hipMemcpyDeviceToHost
#define cudaMemcpyDeviceToDevice hipMemcpyDeviceToDevice
#define cudaMemcpyAttributes hipMemcpyAttributes
#define cudaMemcpySrcAccessOrderStream hipMemcpySrcAccessOrderStream
#define cudaGraph_t hipGraph_t
#define cudaGraphExec_t hipGraphExec_t
#define cudaGraphNode_t hipGraphNode_t
#define cudaGraphDestroy hipGraphDestroy
#define cudaGraphExecDestroy hipGraphExecDestroy
#define cudaGraphLaunch hipGraphLaunch
#define cudaGraphUpload hipGraphUpload
#define cudaGraphGetNodes hipGraphGetNodes
#define cudaGraphNodeGetType hipGraphNodeGetType
#define cudaGraphNodeSetEnabled hipGraphNodeSetEnabled
#define cudaGraphNodeType hipGraphNodeType
#define cudaGraphNodeTypeKernel hipGraphNodeTypeKernel
#define cudaGraphNodeTypeMemcpy hipGraphNodeTypeMemcpy
#define cudaGraphNodeTypeMemset hipGraphNodeTypeMemset
#define cudaGraphAddChildGraphNode hipGraphAddChildGraphNode
#define cudaGraphExecChildGraphNodeSetParams hipGraphExecChildGraphNodeSetParams
#define cudaGraphExecUpdateSuccess hipGraphExecUpdateSuccess
#define cudaFuncAttributeMaxDynamicSharedMemorySize hipFuncAttributeMaxDynamicSharedMemorySize
#define cudaOccupancyMaxActiveBlocksPerMultiprocessor hipOccupancyMaxActiveBlocksPerMultiprocessor
#define cudaLaunchCooperativeKernel hipLaunchCooperativeKernel

template <class Kernel>
inline hipError_t cudaFuncSetAttribute(Kernel kernel, hipFuncAttribute attribute, int value) {
    return hipFuncSetAttribute(reinterpret_cast<const void*>(kernel), attribute, value);
}

// CUDA's newer graph entry points carry edge data absent from HIP 7.2.
// Only the default dependency contract is representable; never drop edge data.
inline hipError_t cudaStreamGetCaptureInfo(
    hipStream_t stream, hipStreamCaptureStatus* status, unsigned long long* id,
    hipGraph_t* graph = nullptr, const hipGraphNode_t** dependencies = nullptr,
    const void** edge_data = nullptr, size_t* count = nullptr) {
    if (edge_data) return hipErrorNotSupported;
    return hipStreamGetCaptureInfo_v2(stream, status, id, graph, dependencies, count);
}
inline hipError_t cudaStreamUpdateCaptureDependencies(
    hipStream_t stream, hipGraphNode_t* dependencies, const void* edge_data,
    size_t count, unsigned int flags = 0) {
    if (edge_data) return hipErrorNotSupported;
    return hipStreamUpdateCaptureDependencies(stream, dependencies, count, flags);
}
inline hipError_t cudaGraphInstantiate(hipGraphExec_t* exec, hipGraph_t graph,
                                      unsigned long long flags = 0) {
    // HIP 7.2 documents that it ignores these flags.
    if (flags) return hipErrorNotSupported;
    return hipGraphInstantiateWithFlags(exec, graph, flags);
}
struct cudaGraphExecUpdateResultInfo {
    hipGraphExecUpdateResult result;
    hipGraphNode_t errorNode;
    hipGraphNode_t errorFromNode;
};
inline hipError_t cudaGraphExecUpdate(hipGraphExec_t exec, hipGraph_t graph,
                                     cudaGraphExecUpdateResultInfo* info) {
    if (!info) return hipErrorInvalidValue;
    info->errorFromNode = nullptr;
    return hipGraphExecUpdate(exec, graph, &info->errorNode, &info->result);
}
inline hipError_t cudaMemcpyBatchAsync(
    void* const* destinations, const void* const* sources, const size_t* sizes, size_t count,
    const hipMemcpyAttributes* attributes, const size_t* indices, size_t attribute_count,
    hipStream_t stream) {
    // HIP 7.2 rejects batch attributes, including the stream source ordering
    // used by Core. Ordered SDK copies implement that contract without hidden
    // storage or synchronization. Reject unsupported attributes before enqueue.
    if (count == 0) return hipSuccess;
    if (!destinations || !sources || !sizes || !attributes || !indices ||
        attribute_count == 0 || attribute_count > count || indices[0] != 0)
        return hipErrorInvalidValue;
    for (size_t i = 0; i < attribute_count; ++i) {
        if (indices[i] >= count || (i && indices[i] <= indices[i - 1]))
            return hipErrorInvalidValue;
        const auto& attribute = attributes[i];
        if (attribute.srcAccessOrder != hipMemcpySrcAccessOrderStream || attribute.flags ||
            attribute.srcLocHint.type != hipMemLocationTypeInvalid ||
            attribute.dstLocHint.type != hipMemLocationTypeInvalid)
            return hipErrorNotSupported;
    }
    for (size_t i = 0; i < count; ++i) {
        const auto error = hipMemcpyAsync(destinations[i], sources[i], sizes[i],
                                          hipMemcpyDefault, stream);
        if (error != hipSuccess) return error;
    }
    return hipSuccess;
}

#else

#include <cuda_runtime.h>
#include <cuda_bf16.h>
#include <cuda_fp16.h>
#include <cuda_fp8.h>

#endif
