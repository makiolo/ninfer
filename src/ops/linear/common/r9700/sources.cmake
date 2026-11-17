# One production compile owner for r9700 linear entries and their routes.
# Consumers can link this family without pulling in unrelated unported Ops.
add_library(ninfer_linear_projection_r9700 SHARED
  "${CMAKE_CURRENT_LIST_DIR}/../../projection/linear.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../../split/linear_split.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../../split/operands.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../../split/bias.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../../split/r9700/dispatch.cu"
  "${CMAKE_CURRENT_LIST_DIR}/../../swiglu/linear_swiglu.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../../swiglu/bias.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../../swiglu/r9700/dispatch.cu"
  "${CMAKE_CURRENT_LIST_DIR}/../../add/linear_add.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../../add/r9700/dispatch.cu"
  "${CMAKE_CURRENT_LIST_DIR}/../../pair/linear_pair.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../../pair/dispatch.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../../pair/r9700/launch.cu"
  "${CMAKE_CURRENT_LIST_DIR}/unsupported.cpp")
ninfer_internal_includes(ninfer_linear_projection_r9700)
ninfer_cuda_library(ninfer_linear_projection_r9700)
set_target_properties(ninfer_linear_projection_r9700 PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_link_libraries(ninfer_linear_projection_r9700 PUBLIC ninfer_core ninfer_ops_host_r9700)
target_link_libraries(ninfer_ops PUBLIC ninfer_linear_projection_r9700)

set(ninfer_linear_projection_target ninfer_linear_projection_r9700)
include("${CMAKE_CURRENT_LIST_DIR}/../../kernels/sources.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../../projection/q4/sources.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../../projection/q5/sources.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../../projection/q8/sources.cmake")
unset(ninfer_linear_projection_target)

target_sources(ninfer_linear_projection_r9700 PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../../topk/r9700/unsupported.cpp")
