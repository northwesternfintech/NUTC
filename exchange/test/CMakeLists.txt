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

add_library(
  NUTC_test_lib OBJECT
  src/test_utils/macros.cpp 
  src/test_utils/process.cpp
  )

add_executable(NUTC_unit_tests
  src/unit/matching/basic_matching.cpp
  src/unit/matching/invalid_orders.cpp
  src/unit/matching/many_orders.cpp

  src/unit/logging/logging_orders.cpp
  )

add_executable(NUTC_integration_tests
  src/integration/basic_rmq.cpp
  )

target_include_directories(
  NUTC_test_lib ${warning_guard}
  PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)

target_link_libraries(
  NUTC_test_lib PRIVATE
  NUTC24_lib
)

target_link_libraries(
  NUTC_unit_tests PRIVATE
  NUTC_test_lib
  NUTC24_lib
  GTest::gtest_main
)

target_link_libraries(
  NUTC_integration_tests PRIVATE 
  NUTC_test_lib
  NUTC24_lib
  GTest::gtest_main
)

target_compile_features(NUTC_test_lib PRIVATE cxx_std_20)
target_compile_features(NUTC_unit_tests PRIVATE cxx_std_20)
target_compile_features(NUTC_integration_tests PRIVATE cxx_std_20)

gtest_discover_tests(NUTC_unit_tests)
gtest_discover_tests(NUTC_integration_tests)

# ---- End-of-file commands ----

add_folders(Test)
