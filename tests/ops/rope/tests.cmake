ninfer_add_op_test(ninfer_rope_test
  SOURCES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/rope.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/rope_large_position.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/rmsnorm_rope_2d.cpp"
  LIBRARIES ninfer_ops)

if(NOT NINFER_TARGET_R9700)
  target_sources(ninfer_rope_test PRIVATE "${CMAKE_CURRENT_LIST_DIR}/rmsnorm_rope.cpp")
endif()
