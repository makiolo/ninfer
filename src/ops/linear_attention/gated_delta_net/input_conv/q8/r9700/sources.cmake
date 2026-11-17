# GDN binds the existing portable Q8 GEMV contraction to its convolution epilogue.
target_sources(ninfer_gdn_r9700 PRIVATE "${CMAKE_CURRENT_LIST_DIR}/launch.cuh")
