if(NINFER_TARGET_R9700)
  include("${CMAKE_CURRENT_LIST_DIR}/r9700/sources.cmake")
  return()
endif()

target_sources(ninfer_ops PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/indexing.cu"
  "${CMAKE_CURRENT_LIST_DIR}/features.cu"
  "${CMAKE_CURRENT_LIST_DIR}/batch_columns.cu"
  "${CMAKE_CURRENT_LIST_DIR}/gather.cu"
  "${CMAKE_CURRENT_LIST_DIR}/state_fields.cu"
)
