#pragma once

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace nutc {
namespace pywrapper {
void init(std::function<bool(const std::string&, int, bool, const std::string&)> publish_market_order);
void create_api_module(
    std::function<bool(const std::string&, int, bool, const std::string&)> publish_market_order
);
} // namespace pywrapper
} // namespace nutc
