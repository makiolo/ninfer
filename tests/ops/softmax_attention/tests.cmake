ninfer_add_op_test(ninfer_sliding_window_attention_test
  SOURCES "${CMAKE_CURRENT_LIST_DIR}/sliding_window_attention.cpp"
  LIBRARIES ninfer_ops)

set(_causal_attention_test_library ninfer_ops)
if(NINFER_TARGET_R9700)
  set(_causal_attention_test_library ninfer_causal_attention_r9700)
endif()
ninfer_add_op_test(ninfer_softmax_attention_test
  SOURCES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/causal/cache.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/causal/device_cache.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/causal/oracle.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/causal/runner.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/causal/cases.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/causal/r9700_cases.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/plain_and_packed.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/context.cpp"
  LIBRARIES ${_causal_attention_test_library}
  FORMATS bf16 int8 fp8 nvfp4 k8v4)
unset(_causal_attention_test_library)

include("${CMAKE_CURRENT_LIST_DIR}/qsa/tests.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/kpool_mla/tests.cmake")
