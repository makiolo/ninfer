#pragma once

#include "core/platform.h"

#if NINFER_TARGET_R9700
#include <hipcub/block/block_merge_sort.hpp>
#include <hipcub/warp/warp_merge_sort.hpp>
#else
#include <cub/block/block_merge_sort.cuh>
#include <cub/warp/warp_merge_sort.cuh>
#endif

namespace ninfer::ops::sampling_detail {
#if NINFER_TARGET_R9700
namespace sort_backend = hipcub;
#else
namespace sort_backend = cub;
#endif

// An empty finite support is an execution failure on either backend.
__device__ __forceinline__ void sampling_execution_failure() {
#if NINFER_TARGET_R9700
    __builtin_trap();
#else
    asm volatile("trap;");
#endif
}
} // namespace ninfer::ops::sampling_detail
