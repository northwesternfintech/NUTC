#pragma once

#include "common/types/decimal.hpp"
#include "exchange/orders/storage/order_storage.hpp"

#include <boost/intrusive/list.hpp>

#include <cassert>

#include <list>

namespace nutc::exchange {

class LimitOrderBook {
    using order_list = std::list<tagged_limit_order>;
    using order_map = std::map<common::decimal_price, order_list>;

    order_map bids_;
    order_map asks_;

public:
    using stored_limit_order = order_list::iterator;

    order_list::iterator add_order(const tagged_limit_order& order);
    void remove_order(order_list::iterator order);
    static void change_quantity(
        order_list::iterator order, common::decimal_quantity quantity_delta
    );

    common::decimal_price get_midprice() const;

    std::optional<LimitOrderBook::stored_limit_order> get_top_order(common::Side side);

private:
    static void remove_order(order_list::iterator order, order_map& map);
};

template <typename T>
inline constexpr bool is_stored_limit_order_v =
    std::is_same_v<std::remove_cvref_t<T>, LimitOrderBook::stored_limit_order>;

} // namespace nutc::exchange
