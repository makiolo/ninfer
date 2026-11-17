cmake_minimum_required(VERSION 3.28)
project(ninfer_host_serve_tests LANGUAGES CXX)

# Standalone, GPU-free gate while the full Engine dependency graph is being ported.
# Compile the production implementation directly; no replacement transport or resolver.
get_filename_component(PROJECT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
include("${PROJECT_SOURCE_DIR}/cmake/NinferTargets.cmake")
find_package(Threads REQUIRED)
if(WIN32)
  find_program(PKG_CONFIG_EXECUTABLE NAMES pkgconf
    PATHS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/pkgconf"
    NO_DEFAULT_PATH REQUIRED)
  set(ENV{PKG_CONFIG_PATH} "")
  set(ENV{PKG_CONFIG_LIBDIR}
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/pkgconfig")
endif()
find_package(PkgConfig REQUIRED)
pkg_check_modules(HWLOC REQUIRED IMPORTED_TARGET hwloc>=2.5)
pkg_check_modules(LIBCURL REQUIRED IMPORTED_TARGET libcurl>=7.85)
add_library(ninfer::json INTERFACE IMPORTED)
target_include_directories(ninfer::json INTERFACE "${PROJECT_SOURCE_DIR}/third_party")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party/xgrammar" xgrammar EXCLUDE_FROM_ALL)

include("${CMAKE_CURRENT_LIST_DIR}/HostTransport.cmake")
ninfer_add_host_transport(ninfer_serve_host_transport)

add_library(ninfer_media_acquire_host STATIC
  "${PROJECT_SOURCE_DIR}/src/product/media_acquire/acquire.cpp")
ninfer_internal_includes(ninfer_media_acquire_host)
target_link_libraries(ninfer_media_acquire_host PRIVATE PkgConfig::LIBCURL)
if(WIN32)
  target_link_libraries(ninfer_media_acquire_host PRIVATE ws2_32)
endif()

enable_testing()
include("${PROJECT_SOURCE_DIR}/tests/cmake/NinferTests.cmake")
ninfer_add_test(ninfer_http_transport_test
  SOURCES "${CMAKE_CURRENT_LIST_DIR}/../test_http_transport.cpp"
  LIBRARIES ninfer_serve_host_transport)
ninfer_add_test(ninfer_http_loopback_test
  SOURCES "${CMAKE_CURRENT_LIST_DIR}/test_http_loopback.cpp"
  LIBRARIES ninfer_serve_host_transport)
ninfer_add_test(ninfer_media_acquire_host_test
  SOURCES "${CMAKE_CURRENT_LIST_DIR}/test_media_acquire.cpp"
  LIBRARIES ninfer_serve_host_transport ninfer_media_acquire_host)
set_tests_properties(ninfer_http_transport_test ninfer_http_loopback_test
  ninfer_media_acquire_host_test PROPERTIES LABELS host TIMEOUT 30)
