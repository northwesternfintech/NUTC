cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    NUTC24
    VERSION 0.1.0
    DESCRIPTION "Northwestern University Trading Competition, 2024"
    HOMEPAGE_URL "https://github.com/northwesternfintech/NUTC"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
    NUTC24_lib OBJECT
    src/lib.cpp
    src/local_algos/sandbox.cpp
    src/local_algos/dev_mode.cpp
    src/local_algos/file_management.cpp
    src/matching/manager/engine_manager.cpp
    src/logging.cpp
    src/curl/curl.cpp
    src/process_spawning/spawning.cpp
    src/rabbitmq/client_manager/RabbitMQClientManager.cpp
    src/rabbitmq/connection_manager/RabbitMQConnectionManager.cpp
    src/rabbitmq/consumer/RabbitMQConsumer.cpp
    src/rabbitmq/order_handler/RabbitMQOrderHandler.cpp
    src/rabbitmq/publisher/RabbitMQPublisher.cpp
    src/rabbitmq/queue_manager/RabbitMQQueueManager.cpp
    src/matching/engine/engine.cpp
    src/client_manager/client_manager.cpp
    src/utils/logger/logger.cpp
)

target_include_directories(
    NUTC24_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)

target_compile_features(NUTC24_lib PUBLIC cxx_std_20)

# argparse
find_package(argparse REQUIRED)
target_link_libraries(NUTC24_lib PUBLIC argparse::argparse)

# libzip
find_package(libzip REQUIRED)
target_link_libraries(NUTC24_lib PUBLIC libzip::zip)

# Fmt
find_package(fmt REQUIRED)
target_link_libraries(NUTC24_lib PUBLIC fmt::fmt)

# quill
find_package(quill REQUIRED)
target_link_libraries(NUTC24_lib PUBLIC quill::quill)

# rabbitmq
find_package(rabbitmq-c REQUIRED)
target_link_libraries(NUTC24_lib PUBLIC rabbitmq::rabbitmq-static)

# libcurl
find_package(CURL REQUIRED)
target_link_libraries(NUTC24_lib PUBLIC CURL::libcurl)

# glaze
find_package(glaze REQUIRED)
target_link_libraries(NUTC24_lib PUBLIC glaze::glaze)

# ---- Declare executable ----

add_executable(NUTC24_exe src/main.cpp)
add_executable(NUTC24::exe ALIAS NUTC24_exe)

set_property(TARGET NUTC24_exe PROPERTY OUTPUT_NAME NUTC24)

target_compile_features(NUTC24_exe PRIVATE cxx_std_20)

target_link_libraries(NUTC24_exe PRIVATE NUTC24_lib)

# quill
target_link_libraries(NUTC24_exe PRIVATE quill::quill)

# fmt
target_link_libraries(NUTC24_exe PRIVATE fmt::fmt)

# libzip
target_link_libraries(NUTC24_exe PUBLIC libzip::zip)

# rabbitmq
find_package(rabbitmq-c REQUIRED)
target_link_libraries(NUTC24_exe PRIVATE rabbitmq::rabbitmq-static)

# curl
find_package(CURL REQUIRED)
target_link_libraries(NUTC24_exe PRIVATE CURL::libcurl)

target_link_libraries(NUTC24_exe PRIVATE argparse::argparse)

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
