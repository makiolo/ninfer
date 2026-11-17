if(NINFER_TARGET_R9700)
  # native_formats.cu requires CSF32/NVFP4 CUDA pipeline and PTX codecs.
  return()
endif()

ninfer_add_op_test(ninfer_native_weight_codec_test
  SOURCES "${CMAKE_CURRENT_LIST_DIR}/native_formats.cu"
  LIBRARIES ninfer_core)
target_compile_options(ninfer_native_weight_codec_test PRIVATE
  $<$<COMPILE_LANGUAGE:CUDA>:--fmad=false>
  $<$<COMPILE_LANGUAGE:CUDA>:-Xcompiler=-fno-fast-math,-ffp-contract=off>)
