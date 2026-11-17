target_sources(ninfer_gdn_r9700 PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/../input_conv.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../operands.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../projected_conv.cu"
  "${CMAKE_CURRENT_LIST_DIR}/plan.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/execute.cu"
)
