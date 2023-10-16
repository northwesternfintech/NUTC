# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(NUTC-clientTests LANGUAGES CXX)

# ---- Dependencies ----

find_package(GTest REQUIRED)
include(GoogleTest)

# ---- Tests ----

add_executable(NUTC-client_test src/NUTC-client_test.cpp)
target_link_libraries(
    NUTC-client_test PRIVATE
    NUTC-client_lib
    GTest::gtest_main
)
target_compile_features(NUTC-client_test PRIVATE cxx_std_20)

gtest_discover_tests(NUTC-client_test)

# ---- End-of-file commands ----

add_folders(Test)
