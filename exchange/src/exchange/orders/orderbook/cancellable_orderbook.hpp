#pragma once

#include <absl/hash/hash.h>
#include <hash_table7.hpp>

#include <cstdint>

#include <vector>

namespace nutc {
namespace matching {

template <typename BaseOrderBookT>
class CancellableOrderBook : public BaseOrderBookT {
public:
    using OrderT = BaseOrderBookT::OrderT;

private:
    std::vector<uint64_t> ioc_order_ids_;

    // Invariant: any order in order_map has a corresponding reference in the queues
    // In other words, when we remove from the queue, we remove from order map as well
    using OrderIdMap = emhash7::HashMap<
        uint64_t, std::reference_wrapper<OrderT>, absl::Hash<uint64_t>>;
    OrderIdMap order_map_;

public:
    bool
    contains_order(uint64_t order_id) const
    {
        return order_map_.contains(order_id);
    }

    OrderT&
    mark_order_removed(uint64_t order_id)
    {
        OrderT& order = order_map_.at(order_id);
        mark_order_removed(order);
        return order;
    }

    OrderT&
    add_order(const OrderT& order)
    {
        if (order.ioc) {
            ioc_order_ids_.push_back(order.order_index);
        }

        OrderT& added_order = BaseOrderBookT::add_order(order);
        order_map_.emplace(order.order_index, added_order);

        return added_order;
    }

    void
    mark_order_removed(OrderT& order)
    {
        order_map_.erase(order.order_index);

        BaseOrderBookT::mark_order_removed(order);
    }

    std::vector<OrderT>
    remove_ioc_orders()
    {
        std::vector<OrderT> orders;

        for (uint64_t order_id : ioc_order_ids_) {
            if (!contains_order(order_id)) {
                continue;
            }

            OrderT& ioc_order = mark_order_removed(order_id);
            orders.push_back(ioc_order);
        }

        ioc_order_ids_.clear();
        return orders;
    }
};

} // namespace matching
} // namespace nutc
