#pragma once

#include "exchange/orders/storage/order_storage.hpp"

#include <absl/hash/hash.h>

#include <cstdint>

#include <vector>

namespace nutc {
namespace matching {
using OrderIdMap = emhash7::HashMap<
    uint64_t, std::reference_wrapper<tagged_limit_order>, absl::Hash<uint64_t>>;

template <typename BaseOrderBookT>
class CancellableOrderBook : public BaseOrderBookT {
    std::vector<uint64_t> ioc_order_ids_;

    // Invariant: any order in order_map has a corresponding reference in the queues
    // In other words, when we remove from the queue, we remove from order map as well
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

    tagged_limit_order&
    add_order(const tagged_limit_order& order) override
    {
        if (order.ioc) {
            ioc_order_ids_.push_back(order.order_index);
        }

        auto& added_order = BaseOrderBookT::add_order(order);
        order_map_.emplace(order.order_index, added_order);

        return added_order;
    }

    void
    mark_order_removed(tagged_limit_order& order) override
    {
        order_map_.erase(order.order_index);

        BaseOrderBookT::mark_order_removed(order);
    }

    std::vector<tagged_limit_order>
    remove_ioc_orders()
    {
        std::vector<tagged_limit_order> orders;

        for (uint64_t order_id : ioc_order_ids_) {
            if (!contains_order(order_id)) {
                continue;
            }

            auto& ioc_order = mark_order_removed(order_id);
            orders.push_back(ioc_order);
        }

        ioc_order_ids_.clear();
        return orders;
    }
};

} // namespace matching
} // namespace nutc
