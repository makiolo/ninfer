add_library(ninfer_product_logging STATIC
  logging.cpp
  configuration.cpp
  pretty_format.cpp
  startup_log.cpp
  process_signals.cpp
  ../statistics.cpp
  ../observations.cpp
)
ninfer_internal_includes(ninfer_product_logging)
target_link_libraries(ninfer_product_logging PUBLIC spdlog::spdlog ninfer::json)
