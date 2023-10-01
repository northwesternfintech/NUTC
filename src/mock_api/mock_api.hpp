#pragma once

#include "logging.hpp"

#include <iostream>
#include <string>

namespace nutc {
namespace mock_api {

std::function<
    bool(const std::string&, const std::string&, float, float)>
getMarketFunc();
}
} // namespace nutc
