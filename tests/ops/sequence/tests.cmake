set(ninfer_sequence_test_library ninfer_ops)
if(NINFER_TARGET_R9700)
  set(ninfer_sequence_test_library ninfer_sequence_indexing_r9700)
endif()

ninfer_add_op_test(ninfer_sequence_test
  SOURCES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/ragged_prefix.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/masked_block.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/autoregressive_round.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/verify.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/causal_pairs.cpp"
  LIBRARIES ${ninfer_sequence_test_library})
unset(ninfer_sequence_test_library)
