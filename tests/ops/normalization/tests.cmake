ninfer_add_op_test(ninfer_normalization_test
  SOURCES "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/l2norm.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/layer_norm.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/rmsnorm.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/rmsnorm_edge.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/gated_rmsnorm.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/fused.cpp"
          "${CMAKE_CURRENT_LIST_DIR}/gathered_features.cpp"
  LIBRARIES ninfer_ops)

if(NOT NINFER_TARGET_R9700)
  target_sources(ninfer_normalization_test PRIVATE "${CMAKE_CURRENT_LIST_DIR}/rmsnorm_pack_tail.cpp")
endif()
