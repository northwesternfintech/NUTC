#pragma once

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <optional>
#include <fmt/format.h>

namespace nutc {
namespace pywrapper {
std::optional<std::string> create_api_module(
    std::function<bool(const std::string&, int, bool, const std::string&)> publish_market_order
);
} // namespace pywrapper
} // namespace nutc
