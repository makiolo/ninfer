# GDN binds the existing portable Q8 GEMV contraction to its convolution epilogue.
target_sources(ninfer_ops PRIVATE "${CMAKE_CURRENT_LIST_DIR}/launch.cuh")
