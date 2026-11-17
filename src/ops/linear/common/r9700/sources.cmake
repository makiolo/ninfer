# One production compile owner for the r9700 projection entry and its routes.
# Consumers can link this family without pulling in unrelated unported Ops.
add_library(ninfer_linear_projection_r9700 SHARED
  "${CMAKE_CURRENT_LIST_DIR}/../../projection/linear.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/unsupported.cpp")
ninfer_internal_includes(ninfer_linear_projection_r9700)
ninfer_cuda_library(ninfer_linear_projection_r9700)
set_target_properties(ninfer_linear_projection_r9700 PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_link_libraries(ninfer_linear_projection_r9700 PUBLIC ninfer_core)
target_link_libraries(ninfer_ops PUBLIC ninfer_linear_projection_r9700)

set(ninfer_linear_projection_target ninfer_linear_projection_r9700)
include("${CMAKE_CURRENT_LIST_DIR}/../../kernels/sources.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../../projection/q4/sources.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../../projection/q5/sources.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../../projection/q8/sources.cmake")
unset(ninfer_linear_projection_target)
