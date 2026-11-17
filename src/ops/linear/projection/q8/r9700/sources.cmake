target_sources(${ninfer_linear_projection_target} PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/shapes.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../shapes/n2048_k4608.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../shapes/n4608_k4608.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../shapes/n5120_k4608.cpp"
)
