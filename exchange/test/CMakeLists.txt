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

add_executable(NUTC24_test 
  src/unit/matching/basic_matching.cpp
  src/unit/matching/invalid_orders.cpp
  src/unit/matching/many_orders.cpp
  src/unit/logging/logging_orders.cpp
  src/test_utils/macros.cpp 
  )

target_include_directories(
  NUTC24_test ${warning_guard}
  PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)

target_link_libraries(
    NUTC24_test PRIVATE
    NUTC24_lib
    GTest::gtest_main
)
target_compile_features(NUTC24_test PRIVATE cxx_std_20)

gtest_discover_tests(NUTC24_test)

# ---- End-of-file commands ----

add_folders(Test)
