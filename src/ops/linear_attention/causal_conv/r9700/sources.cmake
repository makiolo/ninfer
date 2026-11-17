# Scalar, paired and token-parallel four-tap kernels contain no CUDA ISA operations.
target_sources(ninfer_ops PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/../causal_conv1d_silu.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/../launch.cu"
)
