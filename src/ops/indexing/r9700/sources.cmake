# All indexing entries use the shared exact-copy/index kernels on wave32.
# One production owner lets both suites link without unrelated unported Ops.
add_library(ninfer_sequence_indexing_r9700 SHARED
  "${CMAKE_CURRENT_LIST_DIR}/../indexing.cu"
  "${CMAKE_CURRENT_LIST_DIR}/../features.cu"
  "${CMAKE_CURRENT_LIST_DIR}/../batch_columns.cu"
  "${CMAKE_CURRENT_LIST_DIR}/../gather.cu"
  "${CMAKE_CURRENT_LIST_DIR}/../state_fields.cu")
ninfer_internal_includes(ninfer_sequence_indexing_r9700)
ninfer_cuda_library(ninfer_sequence_indexing_r9700)
set_target_properties(ninfer_sequence_indexing_r9700 PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_link_libraries(ninfer_sequence_indexing_r9700 PUBLIC ninfer_core)
target_link_libraries(ninfer_ops PUBLIC ninfer_sequence_indexing_r9700)
