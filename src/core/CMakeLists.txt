add_library(ninfer_cpu SHARED
  cpu/system.cpp
  cpu/task_pool.cpp
)
ninfer_internal_includes(ninfer_cpu)
target_link_libraries(ninfer_cpu PRIVATE PkgConfig::HWLOC Threads::Threads)
if(WIN32)
  set_target_properties(ninfer_cpu PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

if(NINFER_TARGET STREQUAL "rtx5090" OR NINFER_TARGET_R9700)
  add_library(ninfer_cpu_executor SHARED cpu/executor.cpp cpu/x86.cpp)
  ninfer_internal_includes(ninfer_cpu_executor)
  target_link_libraries(ninfer_cpu_executor PUBLIC ninfer_cpu)
  if(WIN32)
    set_target_properties(ninfer_cpu_executor PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
  endif()
endif()

if(WIN32)
  set(ninfer_io_sources io/file_windows.cpp io/read_queue_windows.cpp)
else()
  set(ninfer_io_sources io/file.cpp io/read_queue.cpp)
endif()

add_library(ninfer_core SHARED
  arena.cpp
  buffer.cpp
  buffer.cu
  memory.cpp
  transfer.cpp
  transfer.cu
  device.cu
  device_memory.cpp
  dtype.cpp
  nf4.cpp
  ${ninfer_io_sources}
  gdn_replay_records.cpp
  kda_replay_records.cpp
  cyclic_kv_cache.cpp
  context_pool.cpp
  paged_kv_cache.cpp
  paged_kv_geometry.cpp
  linear_attention_state.cpp
  state_image.cpp
  layout.cpp
  tensor.cpp
  weight_view.cpp
  weight_bank.cpp
  expert_cache.cpp
  decode_graph.cpp
)
ninfer_internal_includes(ninfer_core)
ninfer_cuda_library(ninfer_core)
if(NINFER_TARGET_R9700)
  set_target_properties(ninfer_core PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
  target_link_libraries(ninfer_core PUBLIC ninfer_cpu CUDA::cudart Threads::Threads)
else()
  target_link_libraries(ninfer_core PUBLIC ninfer_cpu CUDA::cudart CUDA::nvtx3 Threads::Threads)
endif()
target_link_libraries(ninfer_core PRIVATE $<$<NOT:$<BOOL:${NINFER_TARGET_R9700}>>:PkgConfig::URING>)
