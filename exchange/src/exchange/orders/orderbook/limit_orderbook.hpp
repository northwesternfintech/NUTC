#pragma once

#include "exchange/orders/storage/order_storage.hpp"
#include "shared/types/decimal_price.hpp"

#include <boost/intrusive/list.hpp>

#include <cassert>

#include <functional>
#include <queue>

namespace nutc::exchange {

class LimitOrderBook {
    std::map<shared::decimal_price, std::queue<tagged_limit_order>> bids_;
    std::map<shared::decimal_price, std::queue<tagged_limit_order>> asks_;

    void clean_tree(shared::Side side);
    tagged_limit_order pop_from_queue(shared::Side side, shared::decimal_price price);

public:
    virtual tagged_limit_order& add_order(const tagged_limit_order& order);
    virtual void mark_order_removed(tagged_limit_order& order);
    virtual void change_quantity(tagged_limit_order& order, double quantity_delta);
    virtual ~LimitOrderBook() = default;

    shared::decimal_price get_midprice() const;

    std::optional<std::reference_wrapper<tagged_limit_order>>
    get_top_order(shared::Side side);
};
} // namespace nutc::exchange
