#pragma once

#include "common/types/ticker.hpp"
#include "common/util.hpp"
#include "exchange/orders/level_tracking/level_update_generator.hpp"
#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "exchange/orders/orderbook/order_id_tracker.hpp"

namespace nutc::exchange {

class CompositeOrderBook {
    using order_list = std::list<tagged_limit_order>;
    LimitOrderBook orderbook_;
    LevelUpdateGenerator level_update_generator_;
    OrderIdTracker order_id_tracker_;

public:
    explicit CompositeOrderBook(common::Ticker ticker) : level_update_generator_(ticker)
    {}

    common::decimal_price
    get_midprice() const
    {
        return orderbook_.get_midprice();
    }

    LevelUpdateGenerator&
    get_update_generator()
    {
        return level_update_generator_;
    }

    order_list::iterator
    add_order(const tagged_limit_order& order)
    {
        auto stored_order = orderbook_.add_order(order);
        level_update_generator_.record_level_change(
            order.side, order.quantity, order.price
        );
        order_id_tracker_.add_order(stored_order);
        return stored_order;
    }

    void
    remove_order(common::order_id_t order_id)
    {
        std::optional<LimitOrderBook::stored_limit_order> order =
            order_id_tracker_.remove_order(order_id);
        if (!order.has_value())
            return;

        level_update_generator_.record_level_change(
            order.value()->side, -order.value()->quantity, order.value()->price
        );
        orderbook_.mark_order_removed(order.value());
    }

    void
    remove_order(order_list::iterator order)

    {
        orderbook_.mark_order_removed(order);
        level_update_generator_.record_level_change(
            order->side, -order->quantity, order->price
        );
        order_id_tracker_.remove_order(order->order_id);
    }

    void
    change_quantity(order_list::iterator order, common::decimal_quantity quantity_delta)
    {
        if (order->quantity + quantity_delta == 0.0) {
            remove_order(order);
            return;
        }
        LimitOrderBook::change_quantity(order, quantity_delta);
        level_update_generator_.record_level_change(
            order->side, quantity_delta, order->price
        );
    }

    std::optional<LimitOrderBook::stored_limit_order>
    get_top_order(common::Side side)
    {
        return orderbook_.get_top_order(side);
    }
};
} // namespace nutc::exchange
