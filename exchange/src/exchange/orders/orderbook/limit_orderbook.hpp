#pragma once

#include "exchange/orders/storage/order_storage.hpp"
#include "shared/types/decimal.hpp"

#include <boost/intrusive/list.hpp>

#include <cassert>

#include <list>

namespace nutc::exchange {

class LimitOrderBook {
    using order_list = std::list<tagged_limit_order>;

    std::map<shared::decimal_price, order_list> bids_;
    std::map<shared::decimal_price, order_list> asks_;

    void clean_tree(shared::Side side);
    tagged_limit_order pop_from_queue(shared::Side side, shared::decimal_price price);

public:
    using stored_limit_order = order_list::iterator;

    virtual order_list::iterator add_order(const tagged_limit_order& order);
    virtual void mark_order_removed(order_list::iterator order);
    virtual void change_quantity(
        order_list::iterator order, shared::decimal_quantity quantity_delta
    );
    static void
    change_quantity(tagged_limit_order& order, shared::decimal_quantity quantity_delta);
    static void change_quantity(
        tagged_market_order& order, shared::decimal_quantity quantity_delta
    );
    virtual ~LimitOrderBook() = default;

    shared::decimal_price get_midprice() const;

    std::optional<LimitOrderBook::stored_limit_order> get_top_order(shared::Side side);
};

template <typename T>
inline constexpr bool is_stored_limit_order_v =
    std::is_same_v<std::remove_cvref_t<T>, std::list<tagged_limit_order>::iterator>;

} // namespace nutc::exchange
