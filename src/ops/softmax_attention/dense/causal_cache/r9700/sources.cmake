# One production owner is shared by the Engine and the isolated Op test target.
add_library(ninfer_causal_attention_r9700 SHARED
  "${CMAKE_CURRENT_LIST_DIR}/../causal_softmax_attention.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/launch.cu"
  "${CMAKE_CURRENT_LIST_DIR}/unsupported.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../../r9700/unsupported.cpp"
)
ninfer_internal_includes(ninfer_causal_attention_r9700)
ninfer_cuda_library(ninfer_causal_attention_r9700)
set_target_properties(ninfer_causal_attention_r9700 PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_link_libraries(ninfer_causal_attention_r9700 PUBLIC ninfer_core ninfer_kv_cache_r9700)
target_link_libraries(ninfer_ops PUBLIC ninfer_causal_attention_r9700)
target_compile_options(ninfer_causal_attention_r9700 PRIVATE
  "$<$<COMPILE_LANGUAGE:HIP>:-fno-fast-math;-ffp-contract=off;-fno-gpu-flush-denormals-to-zero>")
