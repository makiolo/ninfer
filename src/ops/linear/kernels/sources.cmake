if(NINFER_TARGET_R9700)
  target_sources(${ninfer_linear_projection_target} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/q4/r9700/launch.cu"
    "${CMAKE_CURRENT_LIST_DIR}/q5/r9700/launch.cu"
    "${CMAKE_CURRENT_LIST_DIR}/q8/r9700/launch.cu"
  )
  return()
endif()

include("${CMAKE_CURRENT_LIST_DIR}/fp8/sources.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/nvfp4/sources.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/q4/sources.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/q5/sources.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/q6/sources.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/q8/sources.cmake")
target_sources(ninfer_ops PRIVATE "${CMAKE_CURRENT_LIST_DIR}/mxfp8/a8_quantize.cu")
