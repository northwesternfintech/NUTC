#pragma once

#include "logging.hpp"

#include <fmt/format.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

#include <optional>

namespace nutc {
namespace pywrapper {
bool create_api_module(
    std::function<bool(const std::string&, float, bool, const std::string&, float)>
        publish_market_order
);
std::optional<std::string> import_py_code(const std::string& code);

std::optional<std::string> run_initialization();

std::optional<std::string> trigger_callbacks();
} // namespace pywrapper
} // namespace nutc
