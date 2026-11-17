target_sources(ninfer_ops PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/rmsnorm/rmsnorm_rope_2d.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/apply/rope.cpp"
)

target_sources(ninfer_ops_non_rdc PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/apply/dispatch.cu"
  "${CMAKE_CURRENT_LIST_DIR}/rmsnorm/rmsnorm_rope_2d.cu"
)

if(NINFER_TARGET_R9700)
  target_sources(ninfer_ops PRIVATE "${CMAKE_CURRENT_LIST_DIR}/r9700/unsupported.cpp")
else()
  target_sources(ninfer_ops PRIVATE "${CMAKE_CURRENT_LIST_DIR}/rmsnorm/rmsnorm_rope.cu")
endif()
