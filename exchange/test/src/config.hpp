#pragma once
#include "common/types/decimal.hpp"

#include <cstdint>

namespace nutc::test {
constexpr common::decimal_price TEST_STARTING_CAPITAL = 100000.0;
constexpr uint8_t TEST_CLIENT_WAIT_SECS = 1;
constexpr common::decimal_price TEST_ORDER_FEE = .0001;
} // namespace nutc::test
