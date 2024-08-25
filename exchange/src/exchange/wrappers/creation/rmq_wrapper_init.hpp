#pragma once

#include "exchange/traders/trader_types/generic_trader.hpp"

namespace nutc::exchange {

int64_t get_start_time(size_t wait_seconds);

void send_start_time(GenericTrader& trader, int64_t start_time);
} // namespace nutc::exchange
