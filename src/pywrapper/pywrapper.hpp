#pragma once

#include "logging.hpp"
#include "messages.hpp"

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace nutc {
namespace pywrapper {
py::object get_ob_update_function();
py::object get_trade_update_function();
py::object get_account_update_function();
void init(std::function<bool(
              const std::string&, const std::string&, const std::string&, float, float
          )> publish_market_order);
void create_api_module(
    std::function<
        bool(const std::string&, const std::string&, const std::string&, float, float)>
        publish_market_order
);
void run_code_init(const std::string& py_code);
} // namespace pywrapper
} // namespace nutc
