set(NINFER_TARGET rtx5090 CACHE STRING "NInfer execution target")
set_property(CACHE NINFER_TARGET PROPERTY STRINGS rtx5090 dgx_spark r9700)

if(DEFINED NINFER_CONFIGURED_TARGET AND
   NOT NINFER_CONFIGURED_TARGET STREQUAL NINFER_TARGET)
  message(FATAL_ERROR
    "This build directory belongs to ${NINFER_CONFIGURED_TARGET}; "
    "use a separate directory for ${NINFER_TARGET}")
endif()

set(NINFER_TARGET_RTX5090 0)
set(NINFER_TARGET_SPARK 0)
set(NINFER_TARGET_R9700 0)
if(NINFER_TARGET STREQUAL "rtx5090")
  set(NINFER_TARGET_RTX5090 1)
  set(NINFER_CUDA_ARCHITECTURE 120a)
  set(NINFER_PROCESSOR_PATTERN "^(x86_64|amd64|AMD64)$")
elseif(NINFER_TARGET STREQUAL "dgx_spark")
  set(NINFER_TARGET_SPARK 1)
  set(NINFER_CUDA_ARCHITECTURE 121a)
  set(NINFER_PROCESSOR_PATTERN "^(aarch64|arm64|ARM64)$")
elseif(NINFER_TARGET STREQUAL "r9700")
  set(NINFER_TARGET_R9700 1)
  # Resolve offline host dependencies before project() loads the vcpkg toolchain.
  # Cache values (-D / user presets) take precedence over environment defaults.
  set(ninfer_vcpkg_root "G:/vcpkg_root")
  if(NOT "$ENV{VCPKG_ROOT}" STREQUAL "")
    file(TO_CMAKE_PATH "$ENV{VCPKG_ROOT}" ninfer_vcpkg_root)
  endif()
  set(CMAKE_TOOLCHAIN_FILE "${ninfer_vcpkg_root}/scripts/buildsystems/vcpkg.cmake"
    CACHE FILEPATH "r9700 vcpkg toolchain")
  set(ninfer_vcpkg_installed "${ninfer_vcpkg_root}/installed-ninfer-r9700")
  if(NOT "$ENV{NINFER_R9700_VCPKG_INSTALLED_DIR}" STREQUAL "")
    file(TO_CMAKE_PATH "$ENV{NINFER_R9700_VCPKG_INSTALLED_DIR}" ninfer_vcpkg_installed)
  endif()
  set(VCPKG_INSTALLED_DIR "${ninfer_vcpkg_installed}"
    CACHE PATH "r9700 offline host dependencies")
  unset(ninfer_vcpkg_installed)
  unset(ninfer_vcpkg_root)
  # Shared sources use no C++ modules; HIP SDK Clang has no clang-scan-deps.
  # Initialize every C++ target consistently, including builds without presets.
  set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
  set(NINFER_HIP_ARCHITECTURE gfx1201)
  set(NINFER_PROCESSOR_PATTERN "^(x86_64|amd64|AMD64)$")
  # Reject unsupported hosts before project() starts HIP/ROCm detection.
  # CMAKE_HOST_SYSTEM_PROCESSOR may not be initialized until project().
  set(ninfer_host_processor "${CMAKE_HOST_SYSTEM_PROCESSOR}")
  if(NOT ninfer_host_processor)
    cmake_host_system_information(RESULT ninfer_host_processor QUERY OS_PLATFORM)
  endif()
  if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows" OR
     NOT ninfer_host_processor MATCHES "${NINFER_PROCESSOR_PATTERN}")
    message(FATAL_ERROR
      "${NINFER_TARGET} does not support ${CMAKE_HOST_SYSTEM_NAME}/${ninfer_host_processor}")
  endif()
  unset(ninfer_host_processor)
else()
  message(FATAL_ERROR "Unknown NINFER_TARGET '${NINFER_TARGET}'; expected rtx5090, dgx_spark or r9700")
endif()

# GPU compiler detection must see the selected architecture, not a host default.
if(NINFER_TARGET_R9700)
  if(NOT DEFINED CMAKE_HIP_ARCHITECTURES)
    set(CMAKE_HIP_ARCHITECTURES ${NINFER_HIP_ARCHITECTURE}
      CACHE STRING "HIP architectures to build")
  endif()
elseif(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
  set(CMAKE_CUDA_ARCHITECTURES ${NINFER_CUDA_ARCHITECTURE}
    CACHE STRING "CUDA architectures to build")
endif()
function(ninfer_validate_cuda_architecture)
  if(NOT CMAKE_CUDA_ARCHITECTURES STREQUAL NINFER_CUDA_ARCHITECTURE)
    message(FATAL_ERROR
      "${NINFER_TARGET} requires CMAKE_CUDA_ARCHITECTURES=${NINFER_CUDA_ARCHITECTURE}; "
      "got '${CMAKE_CUDA_ARCHITECTURES}'")
  endif()
endfunction()
function(ninfer_validate_hip_architecture)
  if(NOT CMAKE_HIP_ARCHITECTURES STREQUAL NINFER_HIP_ARCHITECTURE)
    message(FATAL_ERROR
      "${NINFER_TARGET} requires CMAKE_HIP_ARCHITECTURES=${NINFER_HIP_ARCHITECTURE}; "
      "got '${CMAKE_HIP_ARCHITECTURES}'")
  endif()
endfunction()
if(NINFER_TARGET_R9700)
  ninfer_validate_hip_architecture()
else()
  ninfer_validate_cuda_architecture()
endif()

function(ninfer_validate_platform)
  # The toolchain file can change architectures during project() initialization.
  if(NINFER_TARGET_R9700)
    ninfer_validate_hip_architecture()
    set(ninfer_system Windows)
    set(ninfer_architecture ${NINFER_HIP_ARCHITECTURE})
    # Clang's 128-bit host arithmetic has 16-byte alignment with the MSVC ABI.
    # Select conforming STL storage alignment consistently across every owner.
    add_compile_definitions(_ENABLE_EXTENDED_ALIGNED_STORAGE)
    # CMake's Windows HIP rules omit the shared-library creation flag. HIP host
    # linkage uses the same MSVC ABI/driver as C++; carry its DLL selection over.
    set(CMAKE_SHARED_LIBRARY_CREATE_HIP_FLAGS
      "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS}" PARENT_SCOPE)
    set(CMAKE_SHARED_MODULE_CREATE_HIP_FLAGS
      "${CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS}" PARENT_SCOPE)
    # CMake's Windows-Clang link rules use -nostdlib. hip-lang requests
    # --rtlib=compiler-rt, but that cannot add builtins under -nostdlib.
    # Supply the compiler's host builtins explicitly for HIP and C++ links,
    # including host tools whose 128-bit arithmetic has no HIP library dependency.
    execute_process(
      COMMAND "${CMAKE_HIP_COMPILER}" --print-libgcc-file-name --rtlib=compiler-rt
      OUTPUT_VARIABLE ninfer_hip_builtins
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_VARIABLE ninfer_hip_builtins_error
      RESULT_VARIABLE ninfer_hip_builtins_result)
    file(TO_CMAKE_PATH "${ninfer_hip_builtins}" ninfer_hip_builtins)
    if(NOT ninfer_hip_builtins_result STREQUAL "0" OR
       NOT IS_ABSOLUTE "${ninfer_hip_builtins}" OR
       NOT EXISTS "${ninfer_hip_builtins}")
      message(FATAL_ERROR
        "r9700 could not locate HIP compiler-rt builtins: "
        "${ninfer_hip_builtins} (${ninfer_hip_builtins_result}) "
        "${ninfer_hip_builtins_error}")
    endif()
    set(CMAKE_HIP_STANDARD_LIBRARIES
      "${CMAKE_HIP_STANDARD_LIBRARIES} \"${ninfer_hip_builtins}\"" PARENT_SCOPE)
    set(CMAKE_CXX_STANDARD_LIBRARIES
      "${CMAKE_CXX_STANDARD_LIBRARIES} \"${ninfer_hip_builtins}\"" PARENT_SCOPE)
  else()
    ninfer_validate_cuda_architecture()
    set(ninfer_system Linux)
    set(ninfer_architecture sm_${NINFER_CUDA_ARCHITECTURE})
  endif()
  if(NOT CMAKE_SYSTEM_NAME STREQUAL ninfer_system OR
     NOT CMAKE_SYSTEM_PROCESSOR MATCHES "${NINFER_PROCESSOR_PATTERN}")
    message(FATAL_ERROR
      "${NINFER_TARGET} does not support ${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}")
  endif()
  set(NINFER_CONFIGURED_TARGET ${NINFER_TARGET} CACHE INTERNAL "Configured NInfer target")
  configure_file(${PROJECT_SOURCE_DIR}/src/core/platform.h.in
    ${PROJECT_BINARY_DIR}/generated/core/platform.h @ONLY)
  if(NINFER_TARGET_R9700)
    # Preserve CUDA include spellings through the shared compatibility owner.
    # Only forward headers whose APIs cuda_compat.h already supplies.
    foreach(ninfer_cuda_header IN ITEMS
        cuda_runtime.h cuda_runtime_api.h cuda_bf16.h cuda_fp16.h cuda_fp8.h
        cuda_profiler_api.h math_constants.h)
      file(CONFIGURE OUTPUT "${PROJECT_BINARY_DIR}/generated/${ninfer_cuda_header}"
        CONTENT "#pragma once\n#include \"core/cuda_compat.h\"\n" @ONLY)
    endforeach()
  endif()
  message(STATUS "NInfer target: ${NINFER_TARGET} (${CMAKE_SYSTEM_PROCESSOR}, ${ninfer_architecture})")
endfunction()
