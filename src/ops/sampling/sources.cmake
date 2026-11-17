set(ninfer_sampling_target ninfer_ops)
if(NINFER_TARGET_R9700)
  # One production owner also lets this suite build independently of other Ops.
  add_library(ninfer_sampling_r9700 SHARED)
  ninfer_internal_includes(ninfer_sampling_r9700)
  ninfer_cuda_library(ninfer_sampling_r9700)
  set_target_properties(ninfer_sampling_r9700 PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
  target_link_libraries(ninfer_sampling_r9700 PUBLIC ninfer_core)
  target_link_libraries(ninfer_ops PUBLIC ninfer_sampling_r9700)
  set(ninfer_sampling_target ninfer_sampling_r9700)
endif()

target_sources(${ninfer_sampling_target} PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/sample/entry.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/sample/launch.cu"
  "${CMAKE_CURRENT_LIST_DIR}/acceptance/selection.cu"
  "${CMAKE_CURRENT_LIST_DIR}/acceptance/greedy_drafts/entry.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/acceptance/greedy_drafts/launch.cu"
  "${CMAKE_CURRENT_LIST_DIR}/token_counts.cu"
)

# R9700 text/MTP supports sample, greedy-draft acceptance and token counts.
# Vocabulary and DFlash2 proposal/acceptance implementations are not compiled.
if(NOT NINFER_TARGET_R9700)
  target_sources(ninfer_ops PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/vocabulary/selection.cu"
    "${CMAKE_CURRENT_LIST_DIR}/vocabulary/decision.cu"
    "${CMAKE_CURRENT_LIST_DIR}/acceptance/sparse_drafts/entry.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/acceptance/sparse_drafts/launch.cu"
    "${CMAKE_CURRENT_LIST_DIR}/candidate_path/entry.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/candidate_path/execute.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/candidate_path/launch.cu")
endif()
unset(ninfer_sampling_target)
