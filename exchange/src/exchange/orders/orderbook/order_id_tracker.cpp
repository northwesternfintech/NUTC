#include "order_id_tracker.hpp"

namespace nutc::exchange {
std::optional<LimitOrderBook::stored_limit_order>
OrderIdTracker::remove_order(common::order_id_t order_id)
{
    auto order_it = order_map_.find(order_id);
    if (order_it == order_map_.end()) {
        return std::nullopt;
    }
    auto order = order_it->second;
    order_map_.erase(order_it);
    return order;
}

void
OrderIdTracker::add_order(LimitOrderBook::stored_limit_order order)
{
    if (order->ioc)
        return;

    if (order_map_.contains(order->order_id)) [[unlikely]]
        throw std::runtime_error("Two orders added to orderbook with same ID");

    order_map_.emplace(order->order_id, order);
}
} // namespace nutc::exchange
