cmake_minimum_required(VERSION 3.28)
project(ninfer_host_system_tests LANGUAGES CXX)

# Standalone host gate: use the production logging target without Engine/media/network deps.
# The regular product build uses this same owner and source manifest.
get_filename_component(PROJECT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
include("${PROJECT_SOURCE_DIR}/cmake/NinferTargets.cmake")
find_package(Threads REQUIRED)
add_library(ninfer::json INTERFACE IMPORTED)
target_include_directories(ninfer::json INTERFACE "${PROJECT_SOURCE_DIR}/third_party")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party/spdlog" spdlog)
add_subdirectory("${PROJECT_SOURCE_DIR}/src/product/logging" logging)
enable_testing()
include("${PROJECT_SOURCE_DIR}/tests/cmake/NinferTests.cmake")
ninfer_add_test(ninfer_pretty_logging_test
  SOURCES "${CMAKE_CURRENT_LIST_DIR}/../test_pretty_logging.cpp"
  LIBRARIES ninfer_product_logging)

add_executable(ninfer_process_signals_test test_process_signals.cpp)
ninfer_test_includes(ninfer_process_signals_test)
target_link_libraries(ninfer_process_signals_test PRIVATE ninfer_product_logging)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(NAME ninfer_process_signals_test
  COMMAND "${Python3_EXECUTABLE}" -B "${CMAKE_CURRENT_LIST_DIR}/test_process_signals.py"
          "$<TARGET_FILE:ninfer_process_signals_test>")
set_tests_properties(ninfer_process_signals_test PROPERTIES TIMEOUT 30)
