#pragma once

#include "exchange/traders/trader_types/generic_trader.hpp"

namespace nutc {
namespace rabbitmq {
int64_t get_start_time(size_t wait_seconds);

void send_start_time(traders::GenericTrader& trader, int64_t start_time);
} // namespace rabbitmq
} // namespace nutc
