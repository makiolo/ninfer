if(NINFER_TARGET_R9700)
  target_sources(ninfer_ops PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/r9700/unsupported.cpp")
  return()
endif()

target_sources(ninfer_ops PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/lookup.cu"
  "${CMAKE_CURRENT_LIST_DIR}/operands.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/gate.cu"
  "${CMAKE_CURRENT_LIST_DIR}/conv.cu"
  "${CMAKE_CURRENT_LIST_DIR}/state_extract.cu")
