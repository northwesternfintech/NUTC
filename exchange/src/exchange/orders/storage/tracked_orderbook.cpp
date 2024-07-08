#include "tracked_orderbook.hpp"

namespace nutc {
namespace matching {
void
TrackedOrderBook::modify_level_(util::Side side, decimal_price price, double delta)
{
    if (level_update_generator_)
        level_update_generator_->record_level_change(side, price, delta);
}

bool
TrackedOrderBook::contains_order(uint64_t order_id)
{
    return order_map_.contains(order_id);
}

stored_order&
TrackedOrderBook::mark_order_removed(uint64_t order_id)
{
    stored_order& order = order_map_.at(order_id);
    mark_order_removed(order);
    return order;
}

stored_order&
TrackedOrderBook::add_order(stored_order order)
{
    // Do not track OB updates from IOC because they will not change their price level
    if (order.ioc) {
        ioc_order_ids_.push_back(order.order_index);
    }
    else {
        modify_level_(order.side, order.price, order.quantity);
    }

    auto& added_order = OrderBook::add_order(order);
    order_map_.emplace(order.order_index, added_order);

    return added_order;
}

void
TrackedOrderBook::mark_order_removed(stored_order& order)
{
    modify_level_(order.side, order.price, -order.quantity);
    order_map_.erase(order.order_index);

    OrderBook::mark_order_removed(order);
}

void
TrackedOrderBook::change_quantity(stored_order& order, double quantity_delta)
{
    modify_level_(order.side, order.price, quantity_delta);
    OrderBook::change_quantity(order, quantity_delta);
}

std::vector<stored_order>
TrackedOrderBook::remove_ioc_orders()
{
    std::vector<stored_order> orders;

    for (uint64_t order_id : ioc_order_ids_) {
        if (!contains_order(order_id))
            continue;

        auto& ioc_order = mark_order_removed(order_id);
        orders.push_back(ioc_order);
    }

    ioc_order_ids_.clear();
    return orders;
}

} // namespace matching
} // namespace nutc
