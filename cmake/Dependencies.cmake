if(NINFER_TARGET STREQUAL "r9700" AND NOT CMAKE_CXX_COMPILER_LOADED)
  # r9700 presets also load this module before project() reads the toolchain.
  # NinferPlatform resolves the default or user-selected toolchain before project().
  if(NOT EXISTS "${CMAKE_TOOLCHAIN_FILE}")
    message(FATAL_ERROR
      "r9700 requires a local vcpkg toolchain; got '${CMAKE_TOOLCHAIN_FILE}'. "
      "Set VCPKG_ROOT or CMAKE_TOOLCHAIN_FILE to an existing installation")
  endif()
  return()
endif()

if(NINFER_TARGET_R9700)
  # Keep HIP's C++ interface on the platform's architecture too. A host-only
  # configure must not need GPU detection or fall back to an SDK default target.
  set(GPU_TARGETS "${NINFER_HIP_ARCHITECTURE}")
  # vcpkg forwards find_package arguments through generated CMake code.
  # Normalize Windows environment paths so backslashes are not parsed as escapes.
  file(TO_CMAKE_PATH "$ENV{HIP_PATH}" ninfer_hip_path)
  file(TO_CMAKE_PATH "$ENV{ROCM_PATH}" ninfer_rocm_path)
  find_package(hip REQUIRED CONFIG HINTS "${ninfer_hip_path}" "${ninfer_rocm_path}")

  # HIP's runtime implements both the CUDA runtime and driver API equivalents.
  # Keep existing consumers' dependency names local to the r9700 build graph.
  foreach(ninfer_cuda_runtime IN ITEMS cudart cuda_driver)
    add_library(CUDA::${ninfer_cuda_runtime} INTERFACE IMPORTED GLOBAL)
    target_link_libraries(CUDA::${ninfer_cuda_runtime} INTERFACE hip::host)
  endforeach()

  # Profiling annotations are explicitly disabled by core/nvtx.h on r9700.
  # Do not invent a CUDA profiling dependency for the HIP build graph.
else()
  find_package(CUDAToolkit REQUIRED)
endif()
find_package(Threads REQUIRED)
if(NINFER_ENABLE_DISTRIBUTED)
  find_path(NCCL_INCLUDE_DIR nccl.h REQUIRED)
  find_library(NCCL_LIBRARY NAMES nccl REQUIRED)
  add_library(ninfer::nccl UNKNOWN IMPORTED GLOBAL)
  set_target_properties(ninfer::nccl PROPERTIES
    IMPORTED_LOCATION "${NCCL_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${NCCL_INCLUDE_DIR}")
endif()
if(NINFER_TARGET_R9700)
  # hwloc provides only pkg-config metadata. Use vcpkg's native host tool and
  # selected configuration, without inheriting a MinGW/system package search.
  find_program(PKG_CONFIG_EXECUTABLE NAMES pkgconf
    PATHS "${VCPKG_INSTALLED_DIR}/${VCPKG_HOST_TRIPLET}/tools/pkgconf"
    NO_DEFAULT_PATH REQUIRED)
  set(ninfer_pkgconfig_prefix "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    string(APPEND ninfer_pkgconfig_prefix "/debug")
  endif()
  set(ENV{PKG_CONFIG_PATH} "")
  set(ENV{PKG_CONFIG_LIBDIR}
    "${ninfer_pkgconfig_prefix}/lib/pkgconfig;${ninfer_pkgconfig_prefix}/share/pkgconfig")
  set(ninfer_pkgconfig_options NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH)
endif()
find_package(PkgConfig REQUIRED)
pkg_check_modules(HWLOC REQUIRED IMPORTED_TARGET ${ninfer_pkgconfig_options} hwloc>=2.5)
if(NOT NINFER_TARGET_R9700)
  pkg_check_modules(URING REQUIRED IMPORTED_TARGET liburing>=2.5)
endif()

# Repository-pinned header dependencies. No configure-time downloads.
add_library(ninfer::json INTERFACE IMPORTED GLOBAL)
target_include_directories(ninfer::json INTERFACE
  ${PROJECT_SOURCE_DIR}/third_party)

if(NINFER_BUILD_ENGINE)
  find_package(JPEG REQUIRED)
  pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET ${ninfer_pkgconfig_options}
    libavformat libavcodec libavutil libswscale)
  add_subdirectory(third_party/llama-jinja EXCLUDE_FROM_ALL)
  add_subdirectory(third_party/xgrammar EXCLUDE_FROM_ALL)
endif()

if(NINFER_BUILD_PRODUCT_SUPPORT)
  add_library(ninfer::toml INTERFACE IMPORTED GLOBAL)
  target_include_directories(ninfer::toml INTERFACE
    ${PROJECT_SOURCE_DIR}/third_party/tomlplusplus)
  target_compile_features(ninfer::toml INTERFACE cxx_std_17)

  # Media acquisition uses CURLOPT_PROTOCOLS_STR and CURLOPT_REDIR_PROTOCOLS_STR,
  # introduced in libcurl 7.85 (not merely the version of the maintainer environment).
  pkg_check_modules(LIBCURL REQUIRED IMPORTED_TARGET ${ninfer_pkgconfig_options} libcurl>=7.85)
  add_library(ninfer::httplib INTERFACE IMPORTED GLOBAL)
  target_include_directories(ninfer::httplib INTERFACE
    ${PROJECT_SOURCE_DIR}/third_party/cpp-httplib)
  add_subdirectory(third_party/spdlog)
endif()
