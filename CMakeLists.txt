cmake_minimum_required(VERSION 3.14)

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

# ---- Declare library ----

add_library(
    NUTC-client_lib OBJECT
    src/lib.cpp
)

target_include_directories(
    NUTC-client_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)

target_compile_features(NUTC-client_lib PUBLIC cxx_std_20)

find_package(fmt REQUIRED)
target_link_libraries(NUTC-client_lib PRIVATE fmt::fmt)

# ---- Declare executable ----

add_executable(NUTC-client_exe src/main.cpp)
add_executable(NUTC-client::exe ALIAS NUTC-client_exe)

set_property(TARGET NUTC-client_exe PROPERTY OUTPUT_NAME NUTC-client)

target_compile_features(NUTC-client_exe PRIVATE cxx_std_20)

target_link_libraries(NUTC-client_exe PRIVATE NUTC-client_lib)

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
