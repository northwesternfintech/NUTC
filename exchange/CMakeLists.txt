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
    
    src/exchange/rabbitmq/trader_manager/RabbitMQTraderManager.cpp
    src/exchange/rabbitmq/connection_manager/RabbitMQConnectionManager.cpp
    src/exchange/rabbitmq/consumer/RabbitMQConsumer.cpp
    src/exchange/rabbitmq/order_handler/RabbitMQOrderHandler.cpp
    src/exchange/rabbitmq/publisher/RabbitMQPublisher.cpp
    src/exchange/rabbitmq/queue_manager/RabbitMQQueueManager.cpp

    src/exchange/bots/bot_container.cpp
    src/exchange/bots/bot_types/retail.cpp

    src/exchange/tickers/engine/engine.cpp
    src/exchange/tickers/engine/order_container.cpp
    src/exchange/tickers/engine/level_update_generator.cpp

    src/exchange/tickers/manager/ticker_manager.cpp
    
    src/exchange/traders/trader_manager.cpp
    src/exchange/traders/trader_types/generic_trader.cpp
    src/exchange/traders/trader_types/bot_trader.cpp
    
    src/exchange/utils/logger/logger.cpp
    src/shared/file_operations/file_operations.cpp
    
    src/exchange/theo/brownian.cpp
    
    src/exchange/tick_manager/tick_manager.cpp
    
    src/exchange/dashboard/state/ticker_state.cpp
    src/exchange/concurrency/pin_threads.cpp
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

# ---- DASHBOARD ----------
option(DASHBOARD "Enable NUTC Dashboard" OFF)

if(DASHBOARD)
  add_definitions(-DDASHBOARD)
  add_library(
      DASHBOARD_lib OBJECT
      src/exchange/dashboard/dashboard.cpp
  )

  target_link_libraries(DASHBOARD_lib PUBLIC EXCHANGE_lib)

  target_include_directories(
      DASHBOARD_lib ${warning_guard}
      PUBLIC
      "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
  )

# ncurses 
  find_package(Curses REQUIRED)
  target_link_libraries(DASHBOARD_lib PUBLIC Curses::Curses)

  target_compile_features(DASHBOARD_lib PUBLIC cxx_std_20)
endif()

# ---- Declare executable ----

add_executable(EXCHANGE_exe src/exchange/main.cpp)
add_executable(EXCHANGE::exe ALIAS EXCHANGE_exe)

set_property(TARGET EXCHANGE_exe PROPERTY OUTPUT_NAME NUTC)

target_compile_features(EXCHANGE_exe PRIVATE cxx_std_20)

target_link_libraries(EXCHANGE_exe PRIVATE EXCHANGE_lib)

if(DASHBOARD)
  target_link_libraries(EXCHANGE_exe PRIVATE DASHBOARD_lib)
  target_link_libraries(EXCHANGE_exe PRIVATE Curses::Curses)
endif()

# quill
target_link_libraries(EXCHANGE_exe PRIVATE quill::quill)


# fmt
target_link_libraries(EXCHANGE_exe PRIVATE fmt::fmt)


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

# Enable lto for release builds
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT error)
if(lto_supported)
  set_target_properties(WRAPPER_lib PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
  set_target_properties(WRAPPER_exe PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
  set_target_properties(EXCHANGE_lib PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
  set_target_properties(EXCHANGE_exe PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
else()
    message(WARNING "LTO is not supported: ${error}")
endif()

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()


# ---- Developer mode ----
if(NOT NUTC24_DEVELOPER_MODE)
message("lol")
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of NUTC24"
  )
endif()

include(cmake/dev-mode.cmake)
