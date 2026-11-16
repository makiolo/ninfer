# A standalone SDK oracle: no dependency on the still-being-ported Core library.
ninfer_add_test(ninfer_hip_numeric_types_test
  SOURCES "${CMAKE_CURRENT_LIST_DIR}/../core/test_hip_numeric_types.cu")
target_compile_options(ninfer_hip_numeric_types_test PRIVATE
  $<$<COMPILE_LANGUAGE:HIP>:-fno-fast-math>
  $<$<COMPILE_LANGUAGE:HIP>:-ffp-contract=off>)
set_tests_properties(ninfer_hip_numeric_types_test PROPERTIES
  SKIP_RETURN_CODE 77 RUN_SERIAL TRUE ENVIRONMENT "HIP_VISIBLE_DEVICES=0")
