target_sources(ninfer_ops_non_rdc PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/paired_concat.cu"
  "${CMAKE_CURRENT_LIST_DIR}/residual_add.cu")

target_sources(ninfer_ops PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/paired_concat.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/residual_add.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/gathered_features.cu"
  "${CMAKE_CURRENT_LIST_DIR}/rmsnorm.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/operands.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/dispatch.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/launch.cu")

if(NINFER_TARGET_R9700)
  target_sources(ninfer_ops PRIVATE "${CMAKE_CURRENT_LIST_DIR}/r9700/unsupported.cpp")
else()
  target_sources(ninfer_ops PRIVATE "${CMAKE_CURRENT_LIST_DIR}/pack_tail.cu")
endif()
