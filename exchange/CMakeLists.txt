cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    NUTC
    VERSION 0.8.0
    DESCRIPTION "Northwestern University Trading Competition"
    HOMEPAGE_URL "https://github.com/northwesternfintech/NUTC"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

option(LOCAL_DEV "Enable local dev endpoints" OFF)

if(LOCAL_DEV)
  add_definitions(-DNUTC_LOCAL_DEV)
endif()

# ---- EXCHANGE ----------

# ---- Declare library ----

add_library(
    EXCHANGE_lib OBJECT
    
    src/exchange/algos/dev_mode/dev_mode.cpp
    src/exchange/algos/normal_mode/normal_mode.cpp
    src/exchange/algos/algo_manager.cpp
    
    src/exchange/logging.cpp

    src/exchange/curl/curl.cpp
    
    src/exchange/wrappers/handle/wrapper_handle.cpp
    src/exchange/wrappers/creation/rmq_wrapper_init.cpp
    src/exchange/wrappers/messaging/async_pipe_runner.cpp
    src/exchange/wrappers/messaging/pipe_reader.cpp
    src/exchange/wrappers/messaging/pipe_writer.cpp
    
    src/exchange/sandbox_server/crow.cpp

    src/exchange/bots/bot_container.cpp
    src/exchange/bots/bot_types/retail.cpp
    src/exchange/bots/variance.cpp
    src/exchange/bots/bot_types/market_maker.cpp

    src/exchange/matching/engine.cpp
    src/exchange/orders/level_tracking/level_quantity_tracker.cpp
    src/exchange/orders/orderbook/tracked_orderbook.cpp
    src/exchange/orders/storage/order_storage.cpp
    src/exchange/orders/orderbook/limit_orderbook.cpp
    src/exchange/orders/level_tracking/level_update_generator.cpp

	src/exchange/matching_cycle/base/base_strategy.cpp
    
    src/exchange/traders/trader_types/generic_trader.cpp
    src/exchange/traders/trader_types/bot_trader.cpp

    src/exchange/theo/brownian.cpp
    
    src/exchange/metrics/on_tick_metrics.cpp

    src/exchange/config/dynamic/argparse.cpp
    src/exchange/config/dynamic/config.cpp
)

target_include_directories(
    EXCHANGE_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)


target_compile_features(EXCHANGE_lib PUBLIC cxx_std_23)

# argparse
find_package(argparse REQUIRED)
target_link_libraries(EXCHANGE_lib PUBLIC argparse::argparse)

# yaml-cpp
find_package(yaml-cpp REQUIRED)
target_link_libraries(EXCHANGE_lib PUBLIC yaml-cpp::yaml-cpp)

# Prometheus
find_package(prometheus-cpp REQUIRED)
target_link_libraries(EXCHANGE_lib PUBLIC prometheus-cpp::prometheus-cpp)

# Crow 
find_package(Crow REQUIRED)
target_link_libraries(EXCHANGE_lib PUBLIC Crow::Crow)

# Fmt
find_package(fmt REQUIRED)
target_link_libraries(EXCHANGE_lib PUBLIC fmt::fmt)

# quill
find_package(quill REQUIRED)
target_link_libraries(EXCHANGE_lib PUBLIC quill::quill)

# libcurl
find_package(CURL REQUIRED)
target_link_libraries(EXCHANGE_lib PUBLIC CURL::libcurl)

# glaze
find_package(glaze REQUIRED)
target_link_libraries(EXCHANGE_lib PUBLIC glaze::glaze)

# boost
find_package(Boost REQUIRED)
target_link_libraries(EXCHANGE_lib PUBLIC boost::boost)

# emhash
find_package(emhash REQUIRED)
target_link_libraries(EXCHANGE_lib PUBLIC emhash::emhash)


add_executable(EXCHANGE_exe src/exchange/main.cpp)
add_executable(EXCHANGE::exe ALIAS EXCHANGE_exe)

set_property(TARGET EXCHANGE_exe PROPERTY OUTPUT_NAME NUTC)

target_compile_features(EXCHANGE_exe PRIVATE cxx_std_23)

target_link_libraries(EXCHANGE_exe PRIVATE EXCHANGE_lib)

# quill
target_link_libraries(EXCHANGE_exe PRIVATE quill::quill)


# fmt
target_link_libraries(EXCHANGE_exe PRIVATE fmt::fmt)

target_link_libraries(EXCHANGE_exe PRIVATE emhash::emhash)

# yaml-cpp
find_package(yaml-cpp REQUIRED)
target_link_libraries(EXCHANGE_exe PRIVATE yaml-cpp::yaml-cpp)


# curl
find_package(CURL REQUIRED)
target_link_libraries(EXCHANGE_exe PRIVATE CURL::libcurl)

target_link_libraries(EXCHANGE_exe PRIVATE argparse::argparse)

# glaze
find_package(glaze REQUIRED)
target_link_libraries(EXCHANGE_exe PRIVATE glaze::glaze)

target_link_libraries(EXCHANGE_exe PRIVATE boost::boost)

target_link_libraries(EXCHANGE_exe PRIVATE Crow::Crow)

# Prometheus
find_package(prometheus-cpp REQUIRED)
target_link_libraries(EXCHANGE_exe PRIVATE prometheus-cpp::prometheus-cpp)

# ---- Declare executable ----

# ---- WRAPPER ----------

add_library(
    WRAPPER_lib OBJECT
    
    src/wrapper/messaging/comms.cpp
    src/wrapper/pywrapper/pywrapper.cpp
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
target_link_libraries(WRAPPER_lib PUBLIC CURL::libcurl)
target_link_libraries(WRAPPER_lib PUBLIC glaze::glaze)
target_link_libraries(WRAPPER_lib PUBLIC pybind11::pybind11)
target_link_libraries(WRAPPER_lib PUBLIC Python3::Python)
target_link_libraries(WRAPPER_lib PRIVATE boost::boost)
target_link_libraries(WRAPPER_lib PUBLIC ${Python3_LIBRARIES})

add_executable(WRAPPER_exe src/wrapper/main.cpp)
add_executable(WRAPPER::exe ALIAS WRAPPER_exe)

set_property(TARGET WRAPPER_exe PROPERTY OUTPUT_NAME WRAPPER)

target_compile_features(WRAPPER_exe PRIVATE cxx_std_23)

target_link_libraries(WRAPPER_exe PRIVATE WRAPPER_lib)
target_link_libraries(WRAPPER_exe PRIVATE fmt::fmt)
target_link_libraries(WRAPPER_exe PRIVATE quill::quill)

target_link_libraries(WRAPPER_exe PRIVATE argparse::argparse)
target_link_libraries(WRAPPER_exe PRIVATE CURL::libcurl)
target_link_libraries(WRAPPER_exe PRIVATE glaze::glaze)
target_link_libraries(WRAPPER_exe PRIVATE pybind11::pybind11)
target_link_libraries(WRAPPER_exe PRIVATE Python3::Python)
target_link_libraries(WRAPPER_exe PRIVATE boost::boost)
target_link_libraries(WRAPPER_exe PRIVATE ${Python3_LIBRARIES})

# ---- SHARED ----------

add_library(SHARED_lib OBJECT     
    src/shared/file_operations/file_operations.cpp
    src/shared/firebase/firebase.cpp
    src/shared/types/ticker.cpp
    src/shared/util.cpp
)

target_include_directories(
    SHARED_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)

target_link_libraries(SHARED_lib PUBLIC fmt::fmt)
target_link_libraries(SHARED_lib PUBLIC glaze::glaze)
target_link_libraries(SHARED_lib PUBLIC yaml-cpp::yaml-cpp)
target_link_libraries(SHARED_lib PUBLIC glaze::glaze)
target_link_libraries(SHARED_lib PUBLIC CURL::libcurl)

target_link_libraries(EXCHANGE_lib PUBLIC SHARED_lib)
target_link_libraries(EXCHANGE_exe PRIVATE SHARED_lib)

target_link_libraries(WRAPPER_lib PUBLIC SHARED_lib)
target_link_libraries(WRAPPER_exe PRIVATE SHARED_lib)

target_compile_features(SHARED_lib PUBLIC cxx_std_23)

# Enable lto for release builds
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT error)
if(lto_supported)
  set_target_properties(WRAPPER_lib PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
  set_target_properties(WRAPPER_exe PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
  set_target_properties(EXCHANGE_lib PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
  set_target_properties(EXCHANGE_exe PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
  set_target_properties(SHARED_lib PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
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
