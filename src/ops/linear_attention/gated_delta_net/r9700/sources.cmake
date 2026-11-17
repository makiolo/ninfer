# Wave32 SIMT recurrence shares the transition, state ownership and replay effects.
# The RTX launchers below are ISA-neutral; its cp.async record and MMA prefill are excluded.
target_sources(ninfer_ops PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/../recurrence/gated_delta_net.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../recurrence/plan.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../recurrence/rtx5090/recurrent.cu"
  "${CMAKE_CURRENT_LIST_DIR}/../replay/fold.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../replay/record.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../replay/rtx5090/fold.cu"
  "${CMAKE_CURRENT_LIST_DIR}/plan.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/record.cu"
)
