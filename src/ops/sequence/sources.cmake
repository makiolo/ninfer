if(NINFER_TARGET_R9700)
  include("${CMAKE_CURRENT_LIST_DIR}/r9700/sources.cmake")
  return()
endif()

target_sources(ninfer_ops PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/causal_pairs.cu"
  "${CMAKE_CURRENT_LIST_DIR}/autoregressive_round.cu"
  "${CMAKE_CURRENT_LIST_DIR}/masked_block.cu"
  "${CMAKE_CURRENT_LIST_DIR}/ragged_prefix.cu"
  "${CMAKE_CURRENT_LIST_DIR}/verify.cu"
)
