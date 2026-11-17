if(NINFER_TARGET_R9700)
  include("${CMAKE_CURRENT_LIST_DIR}/gated_delta_net/r9700/sources.cmake")
else()
  include("${CMAKE_CURRENT_LIST_DIR}/causal_conv/sources.cmake")
  include("${CMAKE_CURRENT_LIST_DIR}/gated_delta_net/sources.cmake")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/kimi_delta_attention/sources.cmake")
