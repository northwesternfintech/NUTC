#pragma once

#include "exchange/traders/trader_types/generic_trader.hpp"

#include <chrono>

namespace nutc::exchange {
std::chrono::high_resolution_clock::time_point get_start_time(size_t wait_seconds);

void send_start_time(
    GenericTrader& trader, std::chrono::high_resolution_clock::time_point start_time
);
} // namespace nutc::exchange
