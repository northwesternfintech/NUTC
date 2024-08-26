#pragma once

#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "order_pair.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/types/decimal_price.hpp"

#include <glaze/util/expected.hpp>

#include <expected>

namespace nutc::exchange {

class Engine {
    using decimal_price = shared::decimal_price;

    decimal_price order_fee_;
    using side = nutc::shared::Side;
    using match = shared::match;

public:
    explicit Engine(decimal_price order_fee = 0.0) : order_fee_(order_fee) {}

    template <TaggedOrder OrderT>
    std::vector<match> match_order(OrderT order, LimitOrderBook& orderbook);

private:
    template <TaggedOrder OrderT>
    glz::expected<match, bool>
    match_incoming_order_(OrderT& aggressive_order, LimitOrderBook& orderbook);

    template <shared::Side AggressiveSide, TaggedOrder OrderT>
    glz::expected<match, bool> match_incoming_order_(
        OrderT& aggressive_order, LimitOrderBook::stored_limit_order passive_order,
        LimitOrderBook& orderbook
    );

    template <shared::Side AggressiveSide, typename OrderPairT>
    glz::expected<match, bool>
    match_orders_(OrderPairT& orders, LimitOrderBook& orderbook);

    enum class MatchFailure { buyer_failure, seller_failure, done_matching };

    template <shared::Side AggressiveSide, typename OrderPairT>
    glz::expected<match, MatchFailure> attempt_match_(OrderPairT& orders);

    decimal_price total_order_cost_(decimal_price price, double quantity) const;
};

} // namespace nutc::exchange
