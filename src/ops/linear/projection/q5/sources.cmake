if(NINFER_TARGET_R9700)
  target_sources(${ninfer_linear_projection_target} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/q5_dispatch.cpp"
  )
  include("${CMAKE_CURRENT_LIST_DIR}/r9700/sources.cmake")
  return()
endif()

target_sources(ninfer_ops PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/q5_dispatch.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/shapes/n1024_k5120.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/shapes/n6144_k5120.cu"
  "${CMAKE_CURRENT_LIST_DIR}/shapes/n7168_k5120.cu"
  "${CMAKE_CURRENT_LIST_DIR}/shapes/n5120_k6144.cu"
  "${CMAKE_CURRENT_LIST_DIR}/shapes/n5120_k17408.cu"
  "${CMAKE_CURRENT_LIST_DIR}/shapes/n1152_k1152.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/shapes/n1152_k4304.cpp"
)
