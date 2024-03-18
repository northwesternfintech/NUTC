#pragma once

#include "order_storage.hpp"
#include "shared/util.hpp"

#include <cassert>

#include <map>
#include <set>
#include <unordered_map>
#include <vector>

namespace nutc {
namespace matching {

class OrderContainer {
public:
    void
    add_order(StoredOrder order)
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

    std::vector<StoredOrder>
    expire_orders(uint64_t tick)
    {
        std::vector<StoredOrder> result;
        for (uint64_t index : orders_by_tick_[tick]) {
            result.push_back(std::move(orders_by_id_.at(index)));
            orders_by_id_.erase(index);
        }
        orders_by_tick_.erase(tick);
        return result;
    }

    void
    modify_order_quantity(uint64_t order_index, double delta)
    {
        StoredOrder& order = get_order_(order_index);
        order.quantity += delta;
        modify_level_(order.side, order.price, delta);
        assert(order.quantity >= 0);
        if (util::is_close_to_zero(order.quantity))
            remove_order(order_index);
    }

    void
    remove_order(uint64_t order_id)
    {
        StoredOrder& order = get_order_(order_id);
        if (order.side == SIDE::BUY) {
            bids_.erase(order_index{order.price, order_id});
        }
        else {
            asks_.erase(order_index{order.price, order_id});
        }
        modify_level_(order.side, order.price, -order.quantity);
        orders_by_id_.erase(order_id);
    }

    double
    get_level(SIDE side, double price) const
    {
        const auto& levels = side == SIDE::BUY ? bid_levels_ : ask_levels_;
        if (levels.find(price) == levels.end()) {
            return 0;
        }
        return levels.at(price);
    }

    const StoredOrder&
    top_order(SIDE side) const
    {
        if (side == SIDE::BUY) {
            assert(!bids_.empty());
            return get_order_(bids_.begin()->index);
        }
        assert(!asks_.empty());
        return get_order_(asks_.begin()->index);
    }

    bool
    can_match_orders() const
    {
        if (bids_.empty() || asks_.empty()) {
            return false;
        }
        return top_order(SIDE::BUY).can_match(top_order(SIDE::SELL));
    }

private:
    const StoredOrder&
    get_order_(uint64_t order_id) const
    {
        assert(orders_by_id_.find(order_id) != orders_by_id_.end());
        return orders_by_id_.at(order_id);
    }

    StoredOrder&
    get_order_(uint64_t order_id)
    {
        assert(orders_by_id_.find(order_id) != orders_by_id_.end());
        return orders_by_id_.at(order_id);
    }

    void
    modify_level_(SIDE side, double price, double qualtity)
    {
        auto& levels = side == SIDE::BUY ? bid_levels_ : ask_levels_;
        levels[price] += qualtity;
        if (levels[price] == 0) {
            levels.erase(price);
        }
    }

    // both map/sort price, order_index
    std::set<order_index, bid_comparator> bids_;
    std::set<order_index, ask_comparator> asks_;

    // order index -> order
    std::unordered_map<uint64_t, StoredOrder> orders_by_id_;

    // tick -> queue of order ids
    std::map<uint64_t, std::vector<uint64_t>> orders_by_tick_;

    std::unordered_map<double, double> bid_levels_;
    std::unordered_map<double, double> ask_levels_;
};
} // namespace matching
} // namespace nutc
