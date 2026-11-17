target_sources(ninfer_ops PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/../input_conv.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../operands.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../projected_conv.cu"
  "${CMAKE_CURRENT_LIST_DIR}/plan.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/execute.cu"
)
