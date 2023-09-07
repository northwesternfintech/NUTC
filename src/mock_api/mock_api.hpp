#pragma once

#include <iostream>
#include <string>

#include "logging.hpp"

namespace nutc {
namespace mock_api {

std::function<bool(const std::string&, float, bool, const std::string&, float)> getMarketFunc();
}
} // namespace nutc

