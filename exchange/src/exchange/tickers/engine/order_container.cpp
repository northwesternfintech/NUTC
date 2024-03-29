#include "order_container.hpp"

namespace nutc {
namespace matching {
const StoredOrder&
OrderContainer::get_top_order(SIDE side) const
{
    if (side == SIDE::BUY) {
        assert(!bids_.empty());
        return get_order_(bids_.begin()->index);
    }
    assert(!asks_.empty());
    return get_order_(asks_.begin()->index);
}

StoredOrder
OrderContainer::remove_order(uint64_t order_id)
{
    StoredOrder order = std::move(get_order_(order_id));
    order_index index{order.price, order_id};
    if (order.side == SIDE::BUY) {
        assert(bids_.find(index) != bids_.end());
        bids_.erase(index);
    }
    else {
        assert(asks_.find(index) != asks_.end());
        asks_.erase(index);
    }
    modify_level_(order.side, order.price, -order.quantity);
    orders_by_id_.erase(order_id);
    return order;
}

void
OrderContainer::modify_order_quantity(uint64_t order_index, double delta)
{
    StoredOrder& order = get_order_(order_index);
    order.quantity += delta;
    modify_level_(order.side, order.price, delta);
    assert(order.quantity >= 0);
    if (util::is_close_to_zero(order.quantity))
        remove_order(order_index);
}

std::vector<StoredOrder>
OrderContainer::expire_orders(uint64_t tick)
{
    if (orders_by_tick_.find(tick) == orders_by_tick_.end()) {
        return {};
    }

    std::vector<StoredOrder> result;
    for (uint64_t index : orders_by_tick_[tick]) {
        if (!order_exists_(index))
            continue;
        StoredOrder removed_order = remove_order(index);
        result.push_back(std::move(removed_order));
    }
    orders_by_tick_.erase(tick);
    return result;
}

void
OrderContainer::add_order(StoredOrder order)
{
    orders_by_tick_[order.tick].push_back(order.order_index);
    if (order.side == SIDE::BUY) {
        bids_.insert(order_index{order.price, order.order_index});
    }
    else {
        asks_.insert(order_index{order.price, order.order_index});
    }
    modify_level_(order.side, order.price, order.quantity);
    orders_by_id_.emplace(order.order_index, std::move(order));
}

} // namespace matching
} // namespace nutc
