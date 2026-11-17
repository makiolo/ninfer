# Append and attention consume one production owner of the paged KV writer.
add_library(ninfer_kv_cache_r9700 SHARED
  "${CMAKE_CURRENT_LIST_DIR}/../kv_cache_append.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../dispatch.cu"
  "${CMAKE_CURRENT_LIST_DIR}/../../kernels/write.cu")
ninfer_internal_includes(ninfer_kv_cache_r9700)
ninfer_cuda_library(ninfer_kv_cache_r9700)
set_target_properties(ninfer_kv_cache_r9700 PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_link_libraries(ninfer_kv_cache_r9700 PUBLIC ninfer_core)
target_link_libraries(ninfer_ops PUBLIC ninfer_kv_cache_r9700)

# The exact codec includes FP32 subnormals and H256's bit 0 -> 7 rounding order.
target_compile_options(ninfer_kv_cache_r9700 PRIVATE
  "$<$<COMPILE_LANGUAGE:HIP>:-fno-fast-math;-ffp-contract=off;-fno-gpu-flush-denormals-to-zero>")
