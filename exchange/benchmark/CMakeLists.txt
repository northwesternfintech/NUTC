project(NUTC24Benchmarks LANGUAGES CXX)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()
# ---- Dependencies ----

find_package(benchmark REQUIRED)

# ---- Benchmarks ----
add_executable(NUTC_benchmarks
  src/level_quantity_tracker.cpp
)

target_include_directories(
  NUTC_benchmarks ${warning_guard}
  PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)

target_link_libraries(
  NUTC_benchmarks PRIVATE
  EXCHANGE_lib 
  SHARED_lib
  benchmark::benchmark
)

# Enable lto for release builds
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT error)
if(lto_supported)
  set_target_properties(NUTC_benchmarks PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
else()
    message(WARNING "LTO is not supported: ${error}")
endif()

target_compile_features(NUTC_benchmarks PRIVATE cxx_std_20)
