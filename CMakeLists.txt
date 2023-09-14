cmake_minimum_required(VERSION 3.14)

include(FetchContent)
include(cmake/prelude.cmake)

project(
    NUTC-client
    VERSION 0.1.0
    DESCRIPTION "Client for the Northwestern University Trading Competition."
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

configure_file(src/config.h.in config.h)

# ---- Load Dependencies ----

# Conan
find_package(fmt REQUIRED)       # String formatting
find_package(quill REQUIRED)     # Logging

find_package(argparse REQUIRED)  # Argument parsing
find_package(CURL REQUIRED)
find_package(glaze REQUIRED)
find_package(Python COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 REQUIRED)
find_package(Crow REQUIRED)

# Git version tracking
FetchContent_Declare(cmake_git_version_tracking
  GIT_REPOSITORY https://github.com/andrew-hardin/cmake-git-version-tracking.git
  GIT_TAG 9b5fc5088b4089ff2adc20d607976b9923e3d737
)
FetchContent_MakeAvailable(cmake_git_version_tracking)

# ---- Declare library ----

add_library(
    NUTC-client_lib OBJECT
    src/firebase/fetching.cpp
    src/pywrapper/runtime.cpp
    src/mock_api/mock_api.cpp
    src/lint/lint.cpp
    # Utils
    src/logging.cpp
)

target_include_directories(
    NUTC-client_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/>"
)

target_compile_features(NUTC-client_lib PUBLIC cxx_std_20)

target_link_libraries(NUTC-client_lib PRIVATE fmt::fmt)
target_link_libraries(NUTC-client_lib PRIVATE quill::quill)
target_link_libraries(NUTC-client_lib PRIVATE CURL::libcurl)
target_link_libraries(NUTC-client_lib PRIVATE glaze::glaze)
target_link_libraries(NUTC-client_lib PRIVATE pybind11::pybind11)
target_link_libraries(NUTC-client_lib PRIVATE Crow::Crow)
target_link_libraries(NUTC-client_lib PRIVATE Python::Python)


# ---- Declare executable ----

add_executable(NUTC-client_exe src/main.cpp)
add_executable(NUTC-client::exe ALIAS NUTC-client_exe)

set_property(TARGET NUTC-client_exe PROPERTY OUTPUT_NAME NUTC-client)

target_compile_features(NUTC-client_exe PRIVATE cxx_std_20)

target_link_libraries(NUTC-client_exe PRIVATE NUTC-client_lib)
target_link_libraries(NUTC-client_exe PRIVATE fmt::fmt)
target_link_libraries(NUTC-client_exe PRIVATE quill::quill)

target_link_libraries(NUTC-client_exe PRIVATE argparse::argparse)
target_link_libraries(NUTC-client_exe PRIVATE cmake_git_version_tracking)
target_link_libraries(NUTC-client_exe PRIVATE CURL::libcurl)
target_link_libraries(NUTC-client_exe PRIVATE glaze::glaze)
target_link_libraries(NUTC-client_exe PRIVATE pybind11::pybind11)
target_link_libraries(NUTC-client_exe PRIVATE Crow::Crow)
target_link_libraries(NUTC-client_exe PRIVATE Python::Python)



# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT NUTC-client_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of NUTC-client"
  )
endif()

include(cmake/dev-mode.cmake)
