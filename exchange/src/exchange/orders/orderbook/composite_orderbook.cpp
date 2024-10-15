#include "composite_orderbook.hpp"

namespace nutc::exchange {
CompositeOrderBook::order_list::iterator
CompositeOrderBook::add_order(const tagged_limit_order& order)
{
    auto stored_order = orderbook_.add_order(order);
    level_update_generator_.record_level_change(
        order.side, order.quantity, order.price
    );
    order_id_tracker_.add_order(stored_order);
    return stored_order;
}

void
CompositeOrderBook::remove_order(common::order_id_t order_id)
{
    auto order_opt = order_id_tracker_.remove_order(order_id);
    if (!order_opt.has_value()) [[unlikely]]
        return;

    auto order_it = order_opt.value();
    level_update_generator_.record_level_change(
        order_it->side, -order_it->quantity, order_it->price
    );
    orderbook_.remove_order(order_it);
}

void
CompositeOrderBook::remove_order(order_list::iterator order)
{
    level_update_generator_.record_level_change(
        order->side, -order->quantity, order->price
    );
    order_id_tracker_.remove_order(order->order_id);
    orderbook_.remove_order(order);
}

void
CompositeOrderBook::change_quantity(
    order_list::iterator order, common::decimal_quantity quantity_delta
)
{
    if (order->quantity + quantity_delta <= 0.0) {
        remove_order(order);
        return;
    }
    level_update_generator_.record_level_change(
        order->side, quantity_delta, order->price
    );
    LimitOrderBook::change_quantity(order, quantity_delta);
}
} // namespace nutc::exchange
