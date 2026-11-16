# Internal headers are private to each compile owner. External headers come from
# the dependency targets (or a component-local include for bundled C sources).
function(ninfer_internal_includes target)
  target_include_directories(${target} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/generated)
endfunction()

function(ninfer_cuda_library target)
  if(NOT NINFER_TARGET_R9700)
    set_target_properties(${target} PROPERTIES
      CUDA_SEPARABLE_COMPILATION ON
      CUDA_RESOLVE_DEVICE_SYMBOLS ON)
    target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-lineinfo>)
  endif()
endfunction()

function(ninfer_cuda_non_rdc_archive target)
  set_target_properties(${target} PROPERTIES
    POSITION_INDEPENDENT_CODE ON)
  if(NOT NINFER_TARGET_R9700)
    set_target_properties(${target} PROPERTIES
      CUDA_SEPARABLE_COMPILATION OFF
      CUDA_RESOLVE_DEVICE_SYMBOLS OFF)
    target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-lineinfo>)
  endif()
endfunction()

# Run after all manifests have registered their sources. Source properties are
# directory-scoped, so apply LANGUAGE in each target's owning directory.
function(ninfer_configure_hip_sources directory)
  if(NOT NINFER_TARGET_R9700)
    return()
  endif()
  get_property(targets DIRECTORY "${directory}" PROPERTY BUILDSYSTEM_TARGETS)
  foreach(target IN LISTS targets)
    get_target_property(sources ${target} SOURCES)
    get_target_property(source_dir ${target} SOURCE_DIR)
    foreach(source IN LISTS sources)
      if(source MATCHES "\\.cu$")
        cmake_path(ABSOLUTE_PATH source BASE_DIRECTORY "${source_dir}" NORMALIZE)
        set_source_files_properties("${source}" TARGET_DIRECTORY ${target}
          PROPERTIES LANGUAGE HIP)
      endif()
    endforeach()
  endforeach()
  get_property(subdirectories DIRECTORY "${directory}" PROPERTY SUBDIRECTORIES)
  foreach(subdirectory IN LISTS subdirectories)
    ninfer_configure_hip_sources("${subdirectory}")
  endforeach()
endfunction()
