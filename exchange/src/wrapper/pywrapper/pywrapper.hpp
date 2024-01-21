#pragma once

#include "wrapper/logging.hpp"
#include "wrapper/util/messages.hpp"

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace nutc {

/**
 * @brief The namespace for the Python wrapper
 *
 * Contains functions to create the Python API module and run the client algorithm
 */
namespace pywrapper {
/**
 * @brief Gets the callback function for orderbook orderbook_update
 *
 * Designed to be triggered by the rabbitmq class when an orderbook update is received
 */
py::object get_ob_update_function();

/**
 * @brief Gets the callback function for trade updates
 *
 * Designed to be triggered by the rabbitmq class when a trade update is received
 */
py::object get_trade_update_function();

/**
 * @brief Gets the callback function for account updates
 *
 * Designed to be triggered by the rabbitmq class when an account update is received
 */
py::object get_account_update_function();

/**
 * @brief Creates the Python API module
 *
 * Creates the Python API module and adds the publish_market_order function to it
 * This allows the client algorithm to place orders with the global function
 * "place_market_order" which is a callback to the rabbitmq class
 *
 * @param publish_market_order The callback function to place market orders
 */
void create_api_module(
    std::function<bool(const std::string&, const std::string&, float, float)>
        publish_market_order
);

/**
 * @brief Runs the initialize() function in the client algorithm
 */
void run_code_init(const std::string& py_code);
} // namespace pywrapper
} // namespace nutc
