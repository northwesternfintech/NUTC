#pragma once

#include "common/types/messages/messages_exchange_to_wrapper.hpp"
#include "exchange/orders/orderbook/composite_orderbook.hpp"

#include <glaze/util/expected.hpp>

#include <expected>

namespace nutc::exchange {

template <TaggedOrder OrderT>
std::vector<common::match> match_order(
    OrderT order, CompositeOrderBook& orderbook, common::decimal_price order_fee = 0.0
);

} // namespace nutc::exchange
