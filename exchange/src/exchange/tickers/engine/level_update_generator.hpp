#pragma once
#include "exchange/tickers/engine/order_container.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"

#include <vector>

namespace nutc {
namespace matching {
using ob_update = messages::orderbook_update;

std::vector<ob_update> get_updates(
    const std::string& ticker, const OrderContainer& before, const OrderContainer& after
);

} // namespace matching
} // namespace nutc
