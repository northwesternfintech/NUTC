#pragma once

#include "exchange/orders/storage/order_storage.hpp"

#include <functional>
#include <queue>

namespace nutc::matching {
// In other words, market order OrderBook
// But MarketOrderOrderBook doesn't have a nice ring to it

struct OrderIndexComparator {
    bool
    operator()(const stored_order& t1, const stored_order& t2)
    {
        return t1.order_index < t2.order_index;
    }
};

class PricelessOrderBook {
    std::priority_queue<stored_order> bid_orders_;
    std::priority_queue<stored_order> ask_orders_;

    std::optional<std::reference_wrapper<const stored_order>>
    get_top_order(util::Side side)
    {
        auto& pqueue = side == util::Side::buy ? bid_orders_ : ask_orders_;

        while (!pqueue.empty() && pqueue.top().was_removed)
            pqueue.pop();

        if (pqueue.empty())
            return std::nullopt;
        else
            return pqueue.top();
    }

    void
    mark_order_removed(stored_order& order)
    {
        order.trader.process_position_change(
            {order.position.side, order.position.ticker, order.position.price,
             -order.position.quantity}
        );

        order.was_removed = true;
    }

    const stored_order&
    add_order(stored_order order)
    {
        order.trader.process_position_change(order);
        assert(
            order.position.price == std::numeric_limits<decimal_price>::max()
            || order.position.price == std::numeric_limits<decimal_price>::min()
        );

        auto& queue =
            order.position.side == util::Side::buy ? bid_orders_ : ask_orders_;
        queue.push(order);
        return queue.top();
    }
};
} // namespace nutc::matching
