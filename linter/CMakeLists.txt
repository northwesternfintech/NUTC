cmake_minimum_required(VERSION 3.14)

include(FetchContent)
include(cmake/prelude.cmake)

project(
    NUTC-linter
    VERSION 0.1.0
    DESCRIPTION "Linter for the Northwestern University Trading Competition."
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

# ---- Declare library ----

add_library(
    NUTC-linter_lib OBJECT 
    src/firebase/fetching.cpp
    src/pywrapper/runtime.cpp
    src/mock_api/mock_api.cpp
    # Utils
    src/logging.cpp
    )
    
add_library(
    NUTC-linter-spawner_lib OBJECT
    src/lint/lint.cpp
    src/firebase/fetching.cpp
    src/pywrapper/runtime.cpp
    src/mock_api/mock_api.cpp
    # Utils
    src/logging.cpp
)

# --- Main executable libs ----

target_include_directories(
    NUTC-linter_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/>"
)

target_compile_features(NUTC-linter_lib PUBLIC cxx_std_20)

target_link_libraries(NUTC-linter_lib PRIVATE fmt::fmt)
target_link_libraries(NUTC-linter_lib PRIVATE quill::quill)
target_link_libraries(NUTC-linter_lib PRIVATE CURL::libcurl)
target_link_libraries(NUTC-linter_lib PRIVATE glaze::glaze)
target_link_libraries(NUTC-linter_lib PRIVATE pybind11::pybind11)
target_link_libraries(NUTC-linter_lib PRIVATE Crow::Crow)
target_link_libraries(NUTC-linter_lib PRIVATE Python::Python)

# ---- The other executable's libs ----

target_include_directories(
    NUTC-linter-spawner_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/>"
)

target_link_libraries(NUTC-linter-spawner_lib PRIVATE quill::quill)
target_link_libraries(NUTC-linter-spawner_lib PRIVATE glaze::glaze)
target_link_libraries(NUTC-linter-spawner_lib PRIVATE pybind11::pybind11)
target_link_libraries(NUTC-linter-spawner_lib PRIVATE CURL::libcurl)

# ---- Declare main executable ----

add_executable(NUTC-linter_exe src/main.cpp)
add_executable(NUTC-linter::exe ALIAS NUTC-linter_exe)

set_property(TARGET NUTC-linter_exe PROPERTY OUTPUT_NAME NUTC-linter)

target_compile_features(NUTC-linter_exe PRIVATE cxx_std_20)

target_link_libraries(NUTC-linter_exe PRIVATE NUTC-linter_lib)
target_link_libraries(NUTC-linter_exe PRIVATE fmt::fmt)
target_link_libraries(NUTC-linter_exe PRIVATE quill::quill)

target_link_libraries(NUTC-linter_exe PRIVATE argparse::argparse)
target_link_libraries(NUTC-linter_exe PRIVATE CURL::libcurl)
target_link_libraries(NUTC-linter_exe PRIVATE glaze::glaze)
target_link_libraries(NUTC-linter_exe PRIVATE pybind11::pybind11)
target_link_libraries(NUTC-linter_exe PRIVATE Crow::Crow)
target_link_libraries(NUTC-linter_exe PRIVATE Python::Python)

# ---- Declare secondary (process spawning) executable ----

add_executable(NUTC-linter-spawner_exe src/spawner/main.cpp)
add_executable(NUTC-linter-spawner::exe ALIAS NUTC-linter-spawner_exe)

set_property(TARGET NUTC-linter-spawner_exe PROPERTY OUTPUT_NAME NUTC-linter-spawner)

target_compile_features(NUTC-linter-spawner_exe PRIVATE cxx_std_20)

target_link_libraries(NUTC-linter-spawner_exe PRIVATE NUTC-linter-spawner_lib)
target_link_libraries(NUTC-linter-spawner_exe PRIVATE CURL::libcurl)
target_link_libraries(NUTC-linter-spawner_exe PRIVATE argparse::argparse)
target_link_libraries(NUTC-linter-spawner_exe PRIVATE pybind11::pybind11)
target_link_libraries(NUTC-linter-spawner_exe PRIVATE quill::quill)
target_link_libraries(NUTC-linter-spawner_exe PRIVATE glaze::glaze)
target_link_libraries(NUTC-linter-spawner_exe PRIVATE Python::Python)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT NUTC-linter_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of NUTC-linter"
  )
endif()

include(cmake/dev-mode.cmake)
