# Shared by the standalone host gate and r9700's ordinary CTest registration.
function(ninfer_add_host_transport target)
  add_library(${target} STATIC
    "${PROJECT_SOURCE_DIR}/src/serve/http_transport.cpp"
    "${PROJECT_SOURCE_DIR}/src/serve/request_validation.cpp"
    "${PROJECT_SOURCE_DIR}/src/text/json_input.cpp")
  ninfer_internal_includes(${target})
  target_include_directories(${target} PUBLIC
    "${PROJECT_SOURCE_DIR}/third_party"
    "${PROJECT_SOURCE_DIR}/third_party/cpp-httplib"
    "${PROJECT_SOURCE_DIR}/third_party/xgrammar/cpp")
  target_link_libraries(${target} PRIVATE Threads::Threads ninfer_xgrammar)
  if(TARGET ninfer_cpu)
    target_link_libraries(${target} PRIVATE ninfer_cpu)
  else()
    # The CXX-only standalone project has no Core/HIP build graph.
    target_sources(${target} PRIVATE
      "${PROJECT_SOURCE_DIR}/src/core/cpu/task_pool.cpp"
      "${PROJECT_SOURCE_DIR}/src/core/cpu/system.cpp")
    target_link_libraries(${target} PRIVATE PkgConfig::HWLOC)
  endif()
endfunction()
