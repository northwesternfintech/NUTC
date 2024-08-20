#pragma once

#include "exchange/orders/orderbook/limit_orderbook.hpp"

namespace nutc {
namespace matching {

class Engine {
    util::decimal_price order_fee;

public:
    explicit Engine(util::decimal_price order_fee = 0.0) : order_fee(order_fee) {}

    std::vector<stored_match> match_orders(LimitOrderBook& orderbook);

private:
    bool order_can_execute_(
        LimitOrderBook& orderbook, tagged_limit_order& buyer, tagged_limit_order& seller
    );

    stored_match
    create_match(const tagged_limit_order& buyer, const tagged_limit_order& seller);

    static void drop_order(LimitOrderBook& orderbook, uint64_t order_index);
};

} // namespace matching
} // namespace nutc
