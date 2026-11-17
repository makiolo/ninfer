# r9700-only registration of GPU-free contracts. Defer until the ordinary test
# manifests have declared every executable, including frontend suites.
function(ninfer_register_host_tests)
  ninfer_add_test(ninfer_artifact_paths_host_test
    SOURCES "${PROJECT_SOURCE_DIR}/tests/host_system/test_artifact_paths.cpp"
    LIBRARIES ninfer_artifact)
  # Reuse the production source manifests in host test archives. Their public
  # Engine/model linkage includes GPU implementations that these contracts do not
  # exercise. No Engine substitute or production dispatch change is introduced.
  if(NINFER_BUILD_ENGINE)
    ninfer_add_test(ninfer_jinja_time_host_test
      SOURCES "${PROJECT_SOURCE_DIR}/tests/host_system/test_jinja_time.cpp"
      LIBRARIES ninfer_jinja ninfer::json)
    set_property(TEST ninfer_jinja_time_host_test APPEND PROPERTY ENVIRONMENT "TZ=UTC0")
    include("${PROJECT_SOURCE_DIR}/tests/host_serve/HostTransport.cmake")
    ninfer_add_host_transport(ninfer_serve_host_transport)
    foreach(owner IN ITEMS ninfer_serve ninfer_model_control ninfer_model_loading)
      get_target_property(owner_sources ${owner} SOURCES)
      get_target_property(owner_directory ${owner} SOURCE_DIR)
      get_target_property(owner_includes ${owner} INCLUDE_DIRECTORIES)
      set(host_sources)
      foreach(source IN LISTS owner_sources)
        cmake_path(ABSOLUTE_PATH source BASE_DIRECTORY "${owner_directory}" NORMALIZE)
        if(owner STREQUAL "ninfer_serve" OR source MATCHES "/frontend/|/prefix_identity\\.cpp$|/vision/window\\.cpp$|/glm5next/vision/control\\.cpp$|/glm5next/config\\.cpp$|/common/dflash/config\\.cpp$|/token_input\\.cpp$|/common/batch\\.cpp$|/common/context/")
          list(APPEND host_sources "${source}")
        endif()
      endforeach()
      add_library(${owner}_host STATIC ${host_sources})
      ninfer_test_includes(${owner}_host)
      target_include_directories(${owner}_host PRIVATE ${owner_includes})
      target_link_libraries(${owner}_host PUBLIC
        ninfer_runtime_support ninfer_product_config ninfer_media_acquire
        ninfer_grammar ninfer_xgrammar ninfer_jinja ninfer_media_decode ninfer_artifact
        ninfer::json ninfer::httplib)
      # COFF COMDATs let the linker discard unreferenced Engine consumers while
      # preserving every production function actually called by the host tests.
      target_compile_options(${owner}_host PRIVATE -ffunction-sections -fdata-sections)
      target_link_options(${owner}_host INTERFACE "LINKER:/OPT:REF")
    endforeach()
    target_link_libraries(ninfer_model_control_host PUBLIC ninfer_model_loading_host)
    target_link_libraries(ninfer_serve_host PUBLIC ninfer_model_control_host)
    add_executable(ninfer_logging_locale_host_test
      "${PROJECT_SOURCE_DIR}/tests/host_system/test_logging_locale.cpp")
    ninfer_test_includes(ninfer_logging_locale_host_test)
    target_link_libraries(ninfer_logging_locale_host_test PRIVATE ninfer_product_logging)
    add_test(NAME ninfer_logging_locale_host_test
      COMMAND "${Python3_EXECUTABLE}" -B
        "${PROJECT_SOURCE_DIR}/tests/host_system/test_logging_locale.py"
        "$<TARGET_FILE:ninfer_logging_locale_host_test>")

    add_executable(ninfer_process_signals_test
      "${PROJECT_SOURCE_DIR}/tests/host_system/test_process_signals.cpp")
    ninfer_test_includes(ninfer_process_signals_test)
    target_link_libraries(ninfer_process_signals_test PRIVATE ninfer_product_logging)
    add_test(NAME ninfer_process_signals_test
      COMMAND "${Python3_EXECUTABLE}" -B
        "${PROJECT_SOURCE_DIR}/tests/host_system/test_process_signals.py"
        "$<TARGET_FILE:ninfer_process_signals_test>")
    add_test(NAME ninfer_tokenizer_host_test
      COMMAND "${Python3_EXECUTABLE}" -B
        "${PROJECT_SOURCE_DIR}/tests/host_system/test_tokenizer.py"
        "$<TARGET_FILE:ninfer_tokenizer_probe>")
  endif()

  set(host_tests
    ninfer_public_api_test
    ninfer_cpu_system_test ninfer_cpu_task_pool_test ninfer_cpu_x86_test
    ninfer_cpu_executor_test ninfer_cpu_tensor_test
    ninfer_device_sync_invalid_test ninfer_device_sync_empty_test
    ninfer_tensor_test ninfer_memory_test ninfer_nf4_test ninfer_nf4_exact_test
    ninfer_read_queue_test ninfer_io_edges_test ninfer_replay_records_test ninfer_context_pool_test
    ninfer_weight_bank_test
    ninfer_artifact_reader_test ninfer_artifact_backing_test
    ninfer_artifact_paths_host_test
    ninfer_context_cost_measure_test)
  if(NINFER_BUILD_ENGINE)
    list(APPEND host_tests
      ninfer_jinja_test ninfer_jinja_time_host_test ninfer_chat_templates_test ninfer_tokenizer_host_test
      ninfer_host_timing_test
      ninfer_scheduler_test ninfer_context_cost_test ninfer_placement_policy_test
      ninfer_resource_manager_test ninfer_context_capacity_test
      ninfer_grammar_test ninfer_regex_choice_test
      ninfer_regex_choice_oracle_test ninfer_json_schema_test ninfer_json_schema_oracle_test
      ninfer_media_decode_test ninfer_prompt_input_test ninfer_pretty_logging_test ninfer_logging_locale_host_test
      ninfer_process_signals_test ninfer_cli_options_test
      ninfer_openai_responses_test ninfer_openai_responses_store_test
      ninfer_anthropic_schema_test ninfer_serve_options_test
      ninfer_metrics_test ninfer_http_transport_test
      ninfer_http_loopback_test ninfer_media_acquire_host_test ninfer_tool_streaming_test
      ninfer_bench_support_test ninfer_vision_window_test ninfer_token_input_test
      ninfer_batch_execution_test ninfer_recovery_test ninfer_context_conversion_test
      ninfer_qwen_frontend_test ninfer_qwen_tool_constraints_test
      ninfer_qwen_tool_schema_oracle_test ninfer_tool_call_parser_test
      ninfer_glm5next_frontend_resources_test ninfer_glm5next_chat_template_test
      ninfer_glm5next_preparation_test ninfer_glm5next_output_test
      ninfer_glm5next_tool_constraints_test ninfer_glm5next_tool_schema_oracle_test)
    # glm5next_protocol and openai_schema require generation_service.cpp's Engine
    # consumers; http_error_handler also pulls HTTP handlers and monitor.cpp.
    # request_log constructs a GenerationHandle and pulls operational_log.cpp.
    # Reactivate after R97-16 (HIP Engine) and R97-31 (complete ninfer_ops linkage).
    # sampling_defaults needs qwen3_5/config.cpp -> qwen/vision/parameters.cpp;
    # the latter also defines prepare_vision and requires ninfer_ops (R97-31).
    # glm5next_media links host providers but needs a PNG-enabled offline FFmpeg
    # dependency tree (R97-07); the current tree was built with --disable-zlib.
  endif()
  set(host_targets ninfer_device_test)
  foreach(test IN LISTS host_tests)
    set_property(TEST ${test} APPEND PROPERTY LABELS host)
    set_property(TEST ${test} APPEND PROPERTY ENVIRONMENT_MODIFICATION
      "PATH=path_list_prepend:${PROJECT_BINARY_DIR}/lib"
      "PATH=path_list_prepend:${PROJECT_BINARY_DIR}/src/core"
      "PATH=path_list_prepend:${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin"
      "PATH=path_list_prepend:$ENV{HIP_PATH}/bin")
    set_property(TEST ${test} APPEND PROPERTY ENVIRONMENT
      "HIP_VISIBLE_DEVICES=-1" "PYTHONUTF8=1" "PYTHONIOENCODING=utf-8")
    if(TARGET ${test})
      list(APPEND host_targets ${test})
      if(NINFER_BUILD_ENGINE)
        get_target_property(libraries ${test} LINK_LIBRARIES)
        if(libraries)
          list(TRANSFORM libraries REPLACE "^ninfer_serve$" "ninfer_serve_host")
          list(TRANSFORM libraries REPLACE "^ninfer_model_control$" "ninfer_model_control_host")
          list(TRANSFORM libraries REPLACE "^ninfer_model_loading$" "ninfer_model_loading_host")
          list(TRANSFORM libraries REPLACE "^ninfer_model_execution$" "ninfer_model_control_host")
          list(TRANSFORM libraries REPLACE "^ninfer_engine$" "ninfer_runtime_support")
          set_property(TARGET ${test} PROPERTY LINK_LIBRARIES "${libraries}")
        endif()
      endif()
    endif()
  endforeach()
  if(NINFER_BUILD_ENGINE)
    foreach(test IN ITEMS ninfer_http_transport_test ninfer_http_loopback_test)
      set_property(TARGET ${test} PROPERTY LINK_LIBRARIES ninfer_serve_host_transport)
    endforeach()
    set_property(TARGET ninfer_media_acquire_host_test PROPERTY LINK_LIBRARIES
      "ninfer_serve_host_transport;ninfer_media_acquire")
  endif()
  set_tests_properties(ninfer_nf4_exact_test PROPERTIES DEPENDS ninfer_nf4_test)
  target_sources(ninfer_nf4_test PRIVATE ${PROJECT_SOURCE_DIR}/tests/host_system/binary_stdio.cpp)
  if(NINFER_BUILD_ENGINE)
    list(APPEND host_targets ninfer_tokenizer_probe)
    set_tests_properties(ninfer_process_signals_test PROPERTIES TIMEOUT 30)
  endif()
  add_custom_target(ninfer_host_tests DEPENDS ${host_targets})
endfunction()

cmake_language(DEFER CALL ninfer_register_host_tests)
