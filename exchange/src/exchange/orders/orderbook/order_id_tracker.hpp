#pragma once

#include "common/util.hpp"
#include "exchange/orders/orderbook/limit_orderbook.hpp"
#include "exchange/orders/storage/order_storage.hpp"

#include <absl/hash/hash.h>
#include <hash_table7.hpp>

#include <cstdint>

namespace nutc::exchange {

class OrderIdTracker {
    // Invariant: any order in order_map has a corresponding reference in the queues
    // In other words, when we remove from the queue, we remove from order map as well
    using OrderIdMap = emhash7::HashMap<
        uint64_t, LimitOrderBook::stored_limit_order, absl::Hash<uint64_t>>;
    OrderIdMap order_map_;

public:
    std::optional<LimitOrderBook::stored_limit_order>
    remove_order(common::order_id_t order_id)
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
    add_order(LimitOrderBook::stored_limit_order order)
    {
        if (!order->ioc) {
            order_map_.emplace(order->order_id, order);
        }
    }
};

} // namespace nutc::exchange
