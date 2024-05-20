#pragma once

#include "order_container.hpp"

namespace nutc {
namespace matching {

class Engine {
    const double ORDER_FEE;

public:
    explicit Engine(double order_fee = 0.0) : ORDER_FEE(order_fee) {}

    std::vector<stored_match>
    match_order(OrderBook& orderbook, const stored_order& order);

private:
    bool order_can_execute_(
        OrderBook& orderbook, const stored_order& buyer, const stored_order& seller
    );

    stored_match create_match(const stored_order& buyer, const stored_order& seller);

    std::vector<stored_match> attempt_matches_(OrderBook& orderbook);
    static void drop_order(OrderBook& orderbook, uint64_t order_index);
};

} // namespace matching
} // namespace nutc
