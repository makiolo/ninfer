# A standalone SDK oracle: no dependency on the still-being-ported Core library.
ninfer_add_test(ninfer_hip_numeric_types_test
  SOURCES "${CMAKE_CURRENT_LIST_DIR}/../core/test_hip_numeric_types.cu")
target_compile_options(ninfer_hip_numeric_types_test PRIVATE
  $<$<COMPILE_LANGUAGE:HIP>:-fno-fast-math>
  $<$<COMPILE_LANGUAGE:HIP>:-ffp-contract=off>)
set_tests_properties(ninfer_hip_numeric_types_test PROPERTIES
  SKIP_RETURN_CODE 77 RUN_SERIAL TRUE ENVIRONMENT "HIP_VISIBLE_DEVICES=0")

# R97-04: compile real device owners directly while the full Core host port is
# pending. The r9700 platform supplies CUDA forwarding headers to all targets.
ninfer_add_test(ninfer_hip_device_smoke_test SOURCES
  "${CMAKE_CURRENT_LIST_DIR}/../core/test_hip_device_smoke.cu"
  "${PROJECT_SOURCE_DIR}/src/core/device.cu"
  "${PROJECT_SOURCE_DIR}/src/core/buffer.cu"
  "${PROJECT_SOURCE_DIR}/src/core/memory.cpp"
  "${PROJECT_SOURCE_DIR}/src/core/transfer.cu"
  LIBRARIES hip::host)
set_tests_properties(ninfer_hip_device_smoke_test PROPERTIES
  SKIP_RETURN_CODE 77 RUN_SERIAL TRUE ENVIRONMENT "HIP_VISIBLE_DEVICES=0"
  FAIL_REGULAR_EXPRESSION "CUDA cleanup failed;CUDA allocation cleanup failed;CUDA stream cleanup failed;cudaFreeHost failed")
