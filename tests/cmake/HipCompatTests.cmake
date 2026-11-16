# A standalone SDK oracle: no dependency on the still-being-ported Core library.
ninfer_add_test(ninfer_hip_numeric_types_test
  SOURCES "${CMAKE_CURRENT_LIST_DIR}/../core/test_hip_numeric_types.cu")
target_compile_options(ninfer_hip_numeric_types_test PRIVATE
  $<$<COMPILE_LANGUAGE:HIP>:-fno-fast-math>
  $<$<COMPILE_LANGUAGE:HIP>:-ffp-contract=off>)
set_tests_properties(ninfer_hip_numeric_types_test PROPERTIES
  SKIP_RETURN_CODE 77 RUN_SERIAL TRUE ENVIRONMENT "HIP_VISIBLE_DEVICES=0")

# R97-04: compile real device owners directly while the full Core host port is
# pending. Keep the temporary CUDA include spelling bridge private to this test;
# its forwarding header supplies cuda_compat.h to unmodified Core translation units.
set(ninfer_hip_smoke_include "${CMAKE_CURRENT_BINARY_DIR}/hip-device-smoke-include")
file(MAKE_DIRECTORY "${ninfer_hip_smoke_include}")
file(WRITE "${ninfer_hip_smoke_include}/cuda_runtime.h"
  "#pragma once\n#include \"core/cuda_compat.h\"\n")
ninfer_add_test(ninfer_hip_device_smoke_test SOURCES
  "${CMAKE_CURRENT_LIST_DIR}/../core/test_hip_device_smoke.cu"
  "${PROJECT_SOURCE_DIR}/src/core/device.cu"
  "${PROJECT_SOURCE_DIR}/src/core/buffer.cu"
  "${PROJECT_SOURCE_DIR}/src/core/memory.cpp"
  "${PROJECT_SOURCE_DIR}/src/core/transfer.cu"
  LIBRARIES hip::host)
# HIP SDK Clang has no clang-scan-deps; these ordinary C++ sources use no modules.
set_target_properties(ninfer_hip_device_smoke_test PROPERTIES CXX_SCAN_FOR_MODULES OFF)
target_include_directories(ninfer_hip_device_smoke_test BEFORE PRIVATE
  "${ninfer_hip_smoke_include}")
set_tests_properties(ninfer_hip_device_smoke_test PROPERTIES
  SKIP_RETURN_CODE 77 RUN_SERIAL TRUE ENVIRONMENT "HIP_VISIBLE_DEVICES=0"
  FAIL_REGULAR_EXPRESSION "CUDA cleanup failed;CUDA allocation cleanup failed;CUDA stream cleanup failed;cudaFreeHost failed")
