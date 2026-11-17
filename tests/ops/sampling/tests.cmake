set(ninfer_sampling_test_library ninfer_ops)
if(NINFER_TARGET_R9700)
  set(ninfer_sampling_test_library ninfer_sampling_r9700)
endif()
ninfer_add_op_test(ninfer_sampling_test
  SOURCES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/sampling.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/accept_greedy.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/r9700_edges.cpp"
  LIBRARIES ${ninfer_sampling_test_library})
unset(ninfer_sampling_test_library)

if(NOT NINFER_TARGET_R9700)
  target_sources(ninfer_sampling_test PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/vocabulary.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/vocabulary_sparse.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/candidate_selector.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/gathered_selector.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/accept_sparse.cpp")
endif()
