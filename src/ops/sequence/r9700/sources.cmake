# All sequence entries use the shared exact-copy/index kernels on wave32.
target_sources(ninfer_sequence_indexing_r9700 PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/../causal_pairs.cu"
  "${CMAKE_CURRENT_LIST_DIR}/../autoregressive_round.cu"
  "${CMAKE_CURRENT_LIST_DIR}/../masked_block.cu"
  "${CMAKE_CURRENT_LIST_DIR}/../ragged_prefix.cu"
  "${CMAKE_CURRENT_LIST_DIR}/../verify.cu")
