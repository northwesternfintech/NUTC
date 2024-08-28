#pragma once

#include "common/util.hpp"
#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "exchange/orders/storage/order_storage.hpp"

#include <absl/hash/hash.h>
#include <hash_table7.hpp>

#include <cstdint>

namespace nutc::exchange {

template <typename BaseOrderBookT>
class CancellableOrderBook : public BaseOrderBookT {
    // Invariant: any order in order_map has a corresponding reference in the queues
    // In other words, when we remove from the queue, we remove from order map as well
    using OrderIdMap = emhash7::HashMap<
        uint64_t, LimitOrderBook::stored_limit_order, absl::Hash<uint64_t>>;
    OrderIdMap order_map_;

public:
    bool
    contains_order(common::order_id_t order_id) const
    {
        return order_map_.contains(order_id);
    }

    std::optional<LimitOrderBook::stored_limit_order>
    mark_order_removed(common::order_id_t order_id)
    {
        auto order_it = order_map_.find(order_id);
        if (order_it == order_map_.end()) {
            return std::nullopt;
        }
        mark_order_removed(order_it);
        return order_it->second;
    }

    LimitOrderBook::stored_limit_order
    add_order(const tagged_limit_order& order) override
    {
        auto added_order = BaseOrderBookT::add_order(order);

        if (!order.ioc) {
            order_map_.emplace(order.order_id, added_order);
        }

        return added_order;
    }

    void
    mark_order_removed(LimitOrderBook::stored_limit_order order) override
    {
        order_map_.erase(order->order_id);

        BaseOrderBookT::mark_order_removed(order);
    }
};

} // namespace nutc::exchange
