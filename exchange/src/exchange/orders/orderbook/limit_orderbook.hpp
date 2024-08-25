#pragma once

#include "exchange/orders/storage/order_storage.hpp"
#include "shared/types/decimal_price.hpp"

#include <boost/intrusive/list.hpp>

#include <cassert>

#include <functional>
#include <queue>

namespace nutc {
namespace matching {

class LimitOrderBook {
    std::map<util::decimal_price, std::queue<tagged_limit_order>> bids_;
    std::map<util::decimal_price, std::queue<tagged_limit_order>> asks_;

    void clean_tree(util::Side side);
    tagged_limit_order pop_from_queue(util::Side side, util::decimal_price price);

public:
    virtual tagged_limit_order& add_order(const tagged_limit_order& order);
    virtual void mark_order_removed(tagged_limit_order& order);
    virtual void change_quantity(tagged_limit_order& order, double quantity_delta);
    virtual ~LimitOrderBook() = default;

    util::decimal_price get_midprice() const;

    std::optional<std::reference_wrapper<tagged_limit_order>>
    get_top_order(util::Side side);
};
} // namespace matching
} // namespace nutc
