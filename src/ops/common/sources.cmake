if(NINFER_TARGET_R9700)
  add_library(ninfer_ops_host_r9700 SHARED
    "${CMAKE_CURRENT_LIST_DIR}/../weight_input.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/host/operands.cpp"
  )
  ninfer_internal_includes(ninfer_ops_host_r9700)
  set_target_properties(ninfer_ops_host_r9700 PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
  target_link_libraries(ninfer_ops_host_r9700 PUBLIC ninfer_core)
  target_link_libraries(ninfer_ops PUBLIC ninfer_ops_host_r9700)
else()
  target_sources(ninfer_ops PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/host/operands.cpp"
  )
endif()
