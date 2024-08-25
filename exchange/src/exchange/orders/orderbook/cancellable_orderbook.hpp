#pragma once

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
        uint64_t, std::reference_wrapper<tagged_limit_order>, absl::Hash<uint64_t>>;
    OrderIdMap order_map_;

public:
    bool
    contains_order(uint64_t order_id) const
    {
        return order_map_.contains(order_id);
    }

    tagged_limit_order&
    mark_order_removed(uint64_t order_id)
    {
        tagged_limit_order& order = order_map_.at(order_id);
        mark_order_removed(order);
        return order;
    }

    virtual tagged_limit_order&
    add_order(const tagged_limit_order& order) override
    {
        tagged_limit_order& added_order = BaseOrderBookT::add_order(order);

        if (!order.ioc) {
            order_map_.emplace(order.order_index, added_order);
        }

        return added_order;
    }

    virtual void
    mark_order_removed(tagged_limit_order& order) override
    {
        order_map_.erase(order.order_index);

        BaseOrderBookT::mark_order_removed(order);
    }
};

} // namespace nutc::exchange
