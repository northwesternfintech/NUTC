# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(NUTC24Tests LANGUAGES CXX)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()
# ---- Dependencies ----

find_package(GTest REQUIRED)
include(GoogleTest)

# ---- Tests ----
add_executable(NUTC_tests
  src/test_utils/macros.cpp 
  src/test_utils/process.cpp

  src/unit/matching/basic_matching.cpp
  src/unit/matching/invalid_orders.cpp
  src/unit/matching/many_orders.cpp

  src/unit/logging/logging_orders.cpp

  src/integration/basic_rmq.cpp
  src/integration/basic_algo.cpp
  )

target_include_directories(
  NUTC_tests ${warning_guard}
  PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)

target_link_libraries(
  NUTC_tests PRIVATE
  NUTC24_lib
  GTest::gtest_main
)

target_compile_features(NUTC_tests PRIVATE cxx_std_20)

gtest_discover_tests(NUTC_tests)

# ---- End-of-file commands ----

add_folders(Test)
