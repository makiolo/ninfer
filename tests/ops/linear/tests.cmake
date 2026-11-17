add_library(ninfer_linear_test_support STATIC
  "${CMAKE_CURRENT_LIST_DIR}/support.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/weights.cpp")
ninfer_test_includes(ninfer_linear_test_support)
ninfer_op_oracle_options(ninfer_linear_test_support)
if(NINFER_TARGET_R9700)
  target_link_libraries(ninfer_linear_test_support PUBLIC
    ninfer_linear_projection_r9700 ninfer_op_test_support)
else()
  target_link_libraries(ninfer_linear_test_support PUBLIC ninfer_ops ninfer_op_test_support)
endif()

function(ninfer_linear_suite family)
  set(target "ninfer_linear_${family}_test")
  set(sources)
  foreach(source IN LISTS ARGN)
    list(APPEND sources "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/${family}/${source}")
  endforeach()
  add_executable(${target} ${sources})
  ninfer_test_includes(${target})
  ninfer_op_oracle_options(${target})
  target_link_libraries(${target} PRIVATE ninfer_linear_test_support)
  if(NINFER_TARGET_R9700 AND family MATCHES "^(topk|bias)$")
    target_link_libraries(${target} PRIVATE ninfer_ops)
  endif()
endfunction()

function(ninfer_linear_format_tests family)
  set(formats ${ARGN})
  if(NINFER_TARGET_R9700)
    if(family STREQUAL "projection")
      set(formats q4 q5 q8)
    elseif(family STREQUAL "split")
      set(formats q4_q5)
    elseif(family STREQUAL "swiglu")
      set(formats q4)
    elseif(family STREQUAL "add")
      set(formats q5)
    elseif(family STREQUAL "pair")
      set(formats q8)
    else()
      set(formats)
    endif()
  endif()
  foreach(format IN LISTS formats)
    set(name "ninfer_linear_${family}_${format}_test")
    add_test(NAME ${name} COMMAND ninfer_linear_${family}_test --format ${format})
    set_tests_properties(${name} PROPERTIES SKIP_RETURN_CODE 77 LABELS "ops")
  endforeach()
endfunction()

ninfer_linear_suite(projection
  test.cpp runner.cpp cases_bf16.cpp cases_q4.cpp cases_q5.cpp cases_q6.cpp cases_q8.cpp
  cases_fp8.cpp cases_nvfp4.cpp cases_bank.cpp cases_native.cpp cases_rowsplit_boundaries.cpp)
ninfer_linear_format_tests(projection bf16 q4 q5 q6 q8 fp8 nvfp4)
if(NINFER_TARGET STREQUAL "dgx_spark")
  ninfer_linear_format_tests(projection mxfp8 fp8_block_fp32 nvfp4_mul)
endif()

ninfer_linear_suite(split
  test.cpp runner.cpp cases_segmented.cpp cases_two_output.cpp cases_multi_output.cpp cases_bank.cpp
  cases_q4_q5_rowsplit.cpp)
ninfer_linear_format_tests(split bf16 q4_q5 q8 fp8 nvfp4)

ninfer_linear_suite(add
  test.cpp runner.cpp cases_bf16.cpp cases_q4.cpp cases_q5.cpp cases_q8.cpp cases_fp8.cpp
  cases_nvfp4.cpp cases_bank.cpp cases_q5_signed.cpp)
ninfer_linear_format_tests(add bf16 q4 q5 q8 fp8 nvfp4)

ninfer_linear_suite(swiglu
  test.cpp runner.cpp cases_bf16.cpp cases_q4.cpp cases_q8.cpp cases_nvfp4.cpp cases_fp8.cpp cases_bank.cpp
  cases_native.cpp cases_q4_gate_extremes.cpp)
ninfer_linear_format_tests(swiglu q4 q8 fp8 nvfp4)
if(NINFER_TARGET STREQUAL "dgx_spark")
  ninfer_linear_format_tests(swiglu bf16 mxfp8 fp8_block_fp32 nvfp4_mul)
endif()

ninfer_linear_suite(pair test.cpp cases_rejections.cpp)
ninfer_linear_format_tests(pair q8)

if(NOT NINFER_TARGET_R9700)
ninfer_linear_suite(topk test.cpp keys.cpp)
ninfer_linear_format_tests(topk q4 q8 fp8)
if(NINFER_TARGET STREQUAL "dgx_spark")
  ninfer_linear_format_tests(topk bf16 mxfp8)
endif()

ninfer_linear_suite(bias test.cpp cases_native.cpp)
if(NINFER_TARGET STREQUAL "dgx_spark")
  ninfer_linear_format_tests(bias bf16 mxfp8 nvfp4_mul)
endif()
endif()
