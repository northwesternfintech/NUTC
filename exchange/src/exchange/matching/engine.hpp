#pragma once

#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "shared/types/decimal_price.hpp"

#include <expected>

namespace nutc {
namespace matching {

class Engine {
    using decimal_price = util::decimal_price;

    decimal_price order_fee;
    using side = nutc::util::Side;

public:
    explicit Engine(decimal_price order_fee = 0.0) : order_fee(order_fee) {}

    std::vector<stored_match>
    match_order(const tagged_limit_order& incoming_order, LimitOrderBook& orderbook);

private:
    stored_match create_match(
        decimal_price price, tagged_limit_order& buyer, tagged_limit_order& seller
    );

    decimal_price
    calculate_order_cost(decimal_price price, double quantity) const
    {
        decimal_price fee{order_fee * price};
        decimal_price price_per = price + fee;
        return price_per * quantity;
    }

    static constexpr double
    order_quantity(const tagged_limit_order& order1, const tagged_limit_order& order2)
    {
        return std::min(order1.quantity, order2.quantity);
    }

    enum no_match_reason { buyer_failure, seller_failure, done_matching };

    struct match_state {
        bool continue_matching;
        std::optional<stored_match> match;
    };

    template <util::Side AggressiveSide>
    std::expected<stored_match, no_match_reason>
    attempt_match(tagged_limit_order& buyer, tagged_limit_order& seller);

    template <util::Side AggressiveSide>
    std::expected<stored_match, bool> match_orders(
        tagged_limit_order& buyer, tagged_limit_order& seller, LimitOrderBook& orderbook
    );

    template <util::Side AggressiveSide>
    std::expected<stored_match, bool> match_incoming_order(
        tagged_limit_order& aggressive_order, tagged_limit_order& passive_order,
        LimitOrderBook& orderbook
    );
};

} // namespace matching
} // namespace nutc
