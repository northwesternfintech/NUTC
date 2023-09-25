# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(NUTC24Tests LANGUAGES CXX)

# ---- Dependencies ----

find_package(GTest REQUIRED)
include(GoogleTest)

# ---- Tests ----

add_executable(NUTC24_test 
  src/basic_matching.cpp)
target_link_libraries(
    NUTC24_test PRIVATE
    NUTC24_lib
    GTest::gtest_main
)
target_compile_features(NUTC24_test PRIVATE cxx_std_20)

gtest_discover_tests(NUTC24_test)

# ---- End-of-file commands ----

add_folders(Test)
