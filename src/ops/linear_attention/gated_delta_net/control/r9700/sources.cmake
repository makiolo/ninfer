target_sources(ninfer_ops PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/../control.cu"
  "${CMAKE_CURRENT_LIST_DIR}/../operands.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../projection.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/plan.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/execute.cu"
)
