cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    NUTC24
    VERSION 0.1.0
    DESCRIPTION "Northwestern University Trading Competition, 2024"
    HOMEPAGE_URL "https://github.com/northwesternfintech/NUTC24"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
    NUTC24_lib OBJECT
    src/lib.cpp
    src/logging.cpp
    src/client/fetching.cpp
    src/client/spawning.cpp
    src/rabbitmq/rabbitmq.cpp
    src/matching/order.cpp
    src/matching/engine.cpp
    src/matching/order.cpp
    src/client_manager/manager.cpp
)

target_include_directories(
    NUTC24_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)

target_compile_features(NUTC24_lib PUBLIC cxx_std_20)

# Fmt
find_package(fmt REQUIRED)
target_link_libraries(NUTC24_lib PRIVATE fmt::fmt)

# quill
find_package(quill REQUIRED)
target_link_libraries(NUTC24_lib PRIVATE quill::quill)

# rabbitmq
find_package(rabbitmq-c REQUIRED)
target_link_libraries(NUTC24_lib PRIVATE rabbitmq::rabbitmq-static)

# libcurl
find_package(CURL REQUIRED)
target_link_libraries(NUTC24_lib PRIVATE CURL::libcurl)

# glaze
find_package(glaze REQUIRED)
target_link_libraries(NUTC24_lib PRIVATE glaze::glaze)

# ---- Declare executable ----

add_executable(NUTC24_exe src/main.cpp)
add_executable(NUTC24::exe ALIAS NUTC24_exe)

set_property(TARGET NUTC24_exe PROPERTY OUTPUT_NAME NUTC24)

target_compile_features(NUTC24_exe PRIVATE cxx_std_20)

target_link_libraries(NUTC24_exe PRIVATE NUTC24_lib)

# quill
target_link_libraries(NUTC24_exe PRIVATE quill::quill)

# rabbitmq
find_package(rabbitmq-c REQUIRED)
target_link_libraries(NUTC24_exe PRIVATE rabbitmq::rabbitmq-static)

# curl
find_package(CURL REQUIRED)
target_link_libraries(NUTC24_exe PRIVATE CURL::libcurl)

# glaze
find_package(glaze REQUIRED)
target_link_libraries(NUTC24_exe PRIVATE glaze::glaze)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT NUTC24_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of NUTC24"
  )
endif()

include(cmake/dev-mode.cmake)
