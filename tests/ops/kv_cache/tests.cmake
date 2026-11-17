set(ninfer_kv_cache_append_test_library ninfer_ops)
if(NINFER_TARGET_R9700)
  set(ninfer_kv_cache_append_test_library ninfer_kv_cache_r9700)
endif()

ninfer_add_op_test(ninfer_kv_cache_append_test
  SOURCES "${CMAKE_CURRENT_LIST_DIR}/append_main.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/append_full.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/append_prefix.cpp"
  LIBRARIES ${ninfer_kv_cache_append_test_library})

ninfer_add_op_test(ninfer_kv_cache_append_r9700_test SOURCES "${CMAKE_CURRENT_LIST_DIR}/append_r9700_edges.cpp" LIBRARIES ${ninfer_kv_cache_append_test_library})
unset(ninfer_kv_cache_append_test_library)

if(NINFER_TARGET_R9700)
  ninfer_add_op_test(ninfer_kv_cache_append_r9700_rejection_test
    SOURCES "${CMAKE_CURRENT_LIST_DIR}/append_r9700_rejection.cpp"
    LIBRARIES ninfer_ops)
endif()

if(NOT NINFER_TARGET_R9700)
ninfer_add_op_test(ninfer_context_kv_materialize_test
  SOURCES "${CMAKE_CURRENT_LIST_DIR}/context_kv_materialize.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/context_kv_materialize_bf16.cpp"
  LIBRARIES ninfer_ops)
endif()
