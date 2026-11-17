# Wave32 SIMT recurrence shares the transition, state ownership and replay effects.
# The RTX launchers below are ISA-neutral; its cp.async record and MMA prefill are excluded.
add_library(ninfer_gdn_r9700 SHARED
  "${CMAKE_CURRENT_LIST_DIR}/../recurrence/gated_delta_net.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../recurrence/plan.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../recurrence/rtx5090/recurrent.cu"
  "${CMAKE_CURRENT_LIST_DIR}/../replay/fold.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../replay/record.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../replay/rtx5090/fold.cu"
  "${CMAKE_CURRENT_LIST_DIR}/plan.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/record.cu"
)
ninfer_internal_includes(ninfer_gdn_r9700)
ninfer_cuda_library(ninfer_gdn_r9700)
set_target_properties(ninfer_gdn_r9700 PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_link_libraries(ninfer_gdn_r9700 PUBLIC ninfer_core ninfer_ops_host_r9700)
target_link_libraries(ninfer_ops PUBLIC ninfer_gdn_r9700)

# Replay/fold promise the same state bits as the recorded trajectory. Preserve
# transition evaluation order and gradual underflow; explicit fmaf stays fused.
target_compile_options(ninfer_gdn_r9700 PRIVATE
  "$<$<COMPILE_LANGUAGE:HIP>:-fno-fast-math;-ffp-contract=off;-fno-gpu-flush-denormals-to-zero>")

include("${CMAKE_CURRENT_LIST_DIR}/../../causal_conv/sources.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../control/sources.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../input_conv/sources.cmake")
