#pragma once

#include "exchange/orders/orderbook/composite_orderbook.hpp"

#include <glaze/util/expected.hpp>

#include <expected>

namespace nutc::exchange {

template <TaggedOrder OrderT>
std::vector<tagged_match> match_order(
    OrderT order, CompositeOrderBook& orderbook, common::decimal_price order_fee = 0.0
);

} // namespace nutc::exchange
