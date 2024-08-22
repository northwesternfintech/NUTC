#pragma once

#include "exchange/orders/storage/order_storage.hpp"
#include "shared/types/decimal_price.hpp"

#include <cassert>

#include <functional>
#include <queue>

namespace nutc {
namespace matching {

class LimitOrderBook {
public:
    using OrderT = tagged_limit_order;

private:
    std::map<util::decimal_price, std::queue<OrderT>> bids_;
    std::map<util::decimal_price, std::queue<OrderT>> asks_;

    void clean_tree(util::Side side);
    OrderT pop_from_queue(util::Side side, util::decimal_price price);

public:
    virtual OrderT& add_order(const OrderT& order);
    virtual void mark_order_removed(OrderT& order);
    virtual void change_quantity(OrderT& order, double quantity_delta);
    virtual ~LimitOrderBook() = default;

    util::decimal_price get_midprice() const;

    std::optional<std::reference_wrapper<OrderT>> get_top_order(util::Side side);
};
} // namespace matching
} // namespace nutc
