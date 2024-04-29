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

  src/unit/matching/invalid_orders.cpp
  src/unit/matching/many_orders.cpp
  
  src/unit/matching/basic_matching.cpp
  src/unit/matching/order_container.cpp

  src/unit/expiration/basic_expiration.cpp

  src/unit/misc/tick_manager.cpp

  src/integration/basic_algo.cpp
  )

target_include_directories(
  NUTC_tests ${warning_guard}
  PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)

target_link_libraries(
  NUTC_tests PRIVATE
  EXCHANGE_lib 
  SHARED_lib
  GTest::gtest_main
)

# Enable lto for release builds
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT error)
if(lto_supported)
  set_target_properties(NUTC_tests PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
else()
    message(WARNING "LTO is not supported: ${error}")
endif()

target_compile_features(NUTC_tests PRIVATE cxx_std_20)

gtest_discover_tests(NUTC_tests)

# ---- End-of-file commands ----

add_folders(Test)
