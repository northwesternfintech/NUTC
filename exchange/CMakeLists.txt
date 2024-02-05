cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    NUTC
    VERSION 0.1.0
    DESCRIPTION "Northwestern University Trading Competition, 2024"
    HOMEPAGE_URL "https://github.com/northwesternfintech/NUTC"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- EXCHANGE ----------

# ---- Declare library ----

add_library(
    EXCHANGE_lib OBJECT
    
    src/exchange/algos/sandbox_mode/sandbox_mode.cpp
    src/exchange/algos/dev_mode/dev_mode.cpp
    src/exchange/algos/normal_mode/normal_mode.cpp
    
    src/exchange/logging.cpp

    src/exchange/curl/curl.cpp
    
    src/exchange/process_spawning/spawning.cpp
    
    src/exchange/rabbitmq/client_manager/RabbitMQClientManager.cpp
    src/exchange/rabbitmq/connection_manager/RabbitMQConnectionManager.cpp
    src/exchange/rabbitmq/consumer/RabbitMQConsumer.cpp
    src/exchange/rabbitmq/order_handler/RabbitMQOrderHandler.cpp
    src/exchange/rabbitmq/publisher/RabbitMQPublisher.cpp
    src/exchange/rabbitmq/queue_manager/RabbitMQQueueManager.cpp

    src/exchange/bot_framework/bot_container.cpp
    
    src/exchange/matching/engine/engine.cpp
    src/exchange/matching/manager/engine_manager.cpp
    
    src/exchange/client_manager/client_manager.cpp
    
    src/exchange/utils/logger/logger.cpp
    src/exchange/utils/file_operations/file_operations.cpp
    
    src/exchange/randomness/brownian.cpp
    
    src/exchange/tick_manager/tick_manager.cpp
)

target_include_directories(
    EXCHANGE_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)

target_compile_features(EXCHANGE_lib PUBLIC cxx_std_20)

# argparse
find_package(argparse REQUIRED)
target_link_libraries(EXCHANGE_lib PUBLIC argparse::argparse)

# libzip
find_package(libzip REQUIRED)
target_link_libraries(EXCHANGE_lib PUBLIC libzip::zip)

# Fmt
find_package(fmt REQUIRED)
target_link_libraries(EXCHANGE_lib PUBLIC fmt::fmt)

# quill
find_package(quill REQUIRED)
target_link_libraries(EXCHANGE_lib PUBLIC quill::quill)

# rabbitmq
find_package(rabbitmq-c REQUIRED)
target_link_libraries(EXCHANGE_lib PUBLIC rabbitmq::rabbitmq-static)

# libcurl
find_package(CURL REQUIRED)
target_link_libraries(EXCHANGE_lib PUBLIC CURL::libcurl)

# glaze
find_package(glaze REQUIRED)
target_link_libraries(EXCHANGE_lib PUBLIC glaze::glaze)

# ---- Declare executable ----

add_executable(EXCHANGE_exe src/exchange/main.cpp)
add_executable(EXCHANGE::exe ALIAS EXCHANGE_exe)

set_property(TARGET EXCHANGE_exe PROPERTY OUTPUT_NAME NUTC)

target_compile_features(EXCHANGE_exe PRIVATE cxx_std_20)

target_link_libraries(EXCHANGE_exe PRIVATE EXCHANGE_lib)

# quill
target_link_libraries(EXCHANGE_exe PRIVATE quill::quill)

# fmt
target_link_libraries(EXCHANGE_exe PRIVATE fmt::fmt)

# libzip
target_link_libraries(EXCHANGE_exe PUBLIC libzip::zip)

# rabbitmq
find_package(rabbitmq-c REQUIRED)
target_link_libraries(EXCHANGE_exe PRIVATE rabbitmq::rabbitmq-static)

# curl
find_package(CURL REQUIRED)
target_link_libraries(EXCHANGE_exe PRIVATE CURL::libcurl)

target_link_libraries(EXCHANGE_exe PRIVATE argparse::argparse)

# glaze
find_package(glaze REQUIRED)
target_link_libraries(EXCHANGE_exe PRIVATE glaze::glaze)

# ---- WRAPPER ----------

add_library(
    WRAPPER_lib OBJECT
    
    src/wrapper/rabbitmq/rabbitmq.cpp
    src/wrapper/firebase/firebase.cpp
    src/wrapper/pywrapper/pywrapper.cpp
    src/wrapper/dev_mode/dev_mode.cpp
    src/wrapper/pywrapper/rate_limiter.cpp
    # Utils
    src/wrapper/logging.cpp
)

target_include_directories(
    WRAPPER_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)

find_package(Python3 3.12 COMPONENTS Interpreter Development EXACT REQUIRED)
find_package(pybind11 REQUIRED)

target_link_libraries(WRAPPER_lib PUBLIC fmt::fmt)
target_link_libraries(WRAPPER_lib PUBLIC quill::quill)

target_link_libraries(WRAPPER_lib PUBLIC argparse::argparse)
target_link_libraries(WRAPPER_lib PUBLIC rabbitmq::rabbitmq-static)
target_link_libraries(WRAPPER_lib PUBLIC CURL::libcurl)
target_link_libraries(WRAPPER_lib PUBLIC glaze::glaze)
target_link_libraries(WRAPPER_lib PUBLIC pybind11::pybind11)
target_link_libraries(WRAPPER_lib PUBLIC Python3::Python)
target_link_libraries(WRAPPER_lib PUBLIC ${Python3_LIBRARIES})

add_executable(WRAPPER_exe src/wrapper/main.cpp)
add_executable(WRAPPER::exe ALIAS WRAPPER_exe)

set_property(TARGET WRAPPER_exe PROPERTY OUTPUT_NAME WRAPPER)

target_compile_features(WRAPPER_exe PRIVATE cxx_std_20)

target_link_libraries(WRAPPER_exe PRIVATE WRAPPER_lib)
target_link_libraries(WRAPPER_exe PRIVATE fmt::fmt)
target_link_libraries(WRAPPER_exe PRIVATE quill::quill)

target_link_libraries(WRAPPER_exe PRIVATE argparse::argparse)
target_link_libraries(WRAPPER_exe PRIVATE rabbitmq::rabbitmq-static)
target_link_libraries(WRAPPER_exe PRIVATE CURL::libcurl)
target_link_libraries(WRAPPER_exe PRIVATE glaze::glaze)
target_link_libraries(WRAPPER_exe PRIVATE pybind11::pybind11)
target_link_libraries(WRAPPER_exe PRIVATE Python3::Python)
target_link_libraries(WRAPPER_exe PRIVATE ${Python3_LIBRARIES})

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
