#pragma once

#include "exchange/orders/storage/order_storage.hpp"

#include <cassert>

#include <functional>
#include <queue>

namespace nutc {
namespace matching {

class LimitOrderBook {
    std::map<decimal_price, std::queue<stored_order>> bids_;
    std::map<decimal_price, std::queue<stored_order>> asks_;

public:
    virtual stored_order& add_order(stored_order order);
    virtual void mark_order_removed(stored_order& order);
    virtual void change_quantity(stored_order& order, double quantity_delta);

    decimal_price get_midprice() const;

    std::optional<std::reference_wrapper<stored_order>> get_top_order(util::Side side);

    virtual ~LimitOrderBook() = default;

private:
    void clean_tree(util::Side side);
    stored_order pop_from_queue(util::Side side, double price);
};
} // namespace matching
} // namespace nutc
