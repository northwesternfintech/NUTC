#pragma once

#include "common/messages_exchange_to_wrapper.hpp"
#include "common/types/decimal.hpp"
#include "exchange/orders/orderbook/composite_orderbook.hpp"
#include "order_pair.hpp"

#include <glaze/util/expected.hpp>

#include <expected>

namespace nutc::exchange {

class Engine {
    using decimal_price = common::decimal_price;

    decimal_price order_fee_;
    using side = nutc::common::Side;
    using match = common::match;

public:
    explicit Engine(decimal_price order_fee = 0.0) : order_fee_(order_fee) {}

    template <TaggedOrder OrderT>
    std::vector<match> match_order(OrderT order, CompositeOrderBook& orderbook);

private:
    template <TaggedOrder OrderT>
    glz::expected<match, bool>
    match_incoming_order_(OrderT& aggressive_order, CompositeOrderBook& orderbook);

    template <common::Side AggressiveSide, TaggedOrder OrderT>
    glz::expected<match, bool> match_incoming_order_(
        OrderT& aggressive_order, LimitOrderBook::stored_limit_order passive_order,
        CompositeOrderBook& orderbook
    );

    template <common::Side AggressiveSide, typename OrderPairT>
    glz::expected<match, bool>
    match_orders_(OrderPairT& orders, CompositeOrderBook& orderbook);

    enum class MatchFailure { buyer_failure, seller_failure, done_matching };

    template <common::Side AggressiveSide, typename OrderPairT>
    glz::expected<match, MatchFailure> attempt_match_(OrderPairT& orders);

    decimal_price
    total_order_cost_(decimal_price price, common::decimal_quantity quantity) const;
};

} // namespace nutc::exchange
