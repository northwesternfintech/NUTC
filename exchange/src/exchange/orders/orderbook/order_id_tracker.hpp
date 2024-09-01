#pragma once

#include "common/util.hpp"
#include "exchange/orders/orderbook/limit_orderbook.hpp"

#include <absl/hash/hash.h>
#include <hash_table7.hpp>

#include <cstdint>

namespace nutc::exchange {

class OrderIdTracker {
    // Invariant: any order in order_map has a corresponding reference in the queues
    // In other words, when we remove from the queue, we remove from order map as well
    emhash7::HashMap<uint64_t, LimitOrderBook::stored_limit_order, absl::Hash<uint64_t>>
        order_map_;

public:
    std::optional<LimitOrderBook::stored_limit_order>
    remove_order(common::order_id_t order_id);

    void add_order(LimitOrderBook::stored_limit_order order);
};

} // namespace nutc::exchange
