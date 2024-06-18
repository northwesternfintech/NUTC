# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(NUTC-linterTests LANGUAGES CXX)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()


# ---- Dependencies ----

find_package(GTest REQUIRED)
include(GoogleTest)

# ---- Tests ----

add_executable(NUTC-linter_test src/NUTC-linter_test.cpp)
target_include_directories(
  NUTC-linter_test ${warning_guard}
  PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/>"
  )
target_link_libraries(
    NUTC-linter_test PRIVATE
    NUTC-linter_lib
    GTest::gtest_main
)
target_compile_features(NUTC-linter_test PRIVATE cxx_std_20)

gtest_discover_tests(NUTC-linter_test)

# ---- End-of-file commands ----

add_folders(Test)
