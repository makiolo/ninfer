set(ninfer_indexing_test_library ninfer_ops)
if(NINFER_TARGET_R9700)
  set(ninfer_indexing_test_library ninfer_sequence_indexing_r9700)
endif()

ninfer_add_op_test(ninfer_indexing_test
  SOURCES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/scatter.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/scatter_bf16_batch.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/position.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/features.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/gather_remap.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/scatter_columns.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/gather_rows.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/state_fields.cpp"
  LIBRARIES ${ninfer_indexing_test_library})
unset(ninfer_indexing_test_library)
