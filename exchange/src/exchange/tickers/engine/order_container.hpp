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
    // both map/sort price, order_index
    std::set<order_index, bid_comparator> bids_;
    std::set<order_index, ask_comparator> asks_;

    // order index -> order
    std::unordered_map<uint64_t, StoredOrder> orders_by_id_;

    // tick -> queue of order ids
    std::map<uint64_t, std::vector<uint64_t>> orders_by_tick_;

    std::unordered_map<double, double> bid_levels_;
    std::unordered_map<double, double> ask_levels_;

public:
    /**
     * @brief Get the price->quantity map for a SIDE
     */
    const std::unordered_map<double, double>&
    get_levels(SIDE side) const
    {
        return side == SIDE::BUY ? bid_levels_ : ask_levels_;
    }

    /**
     * @brief Get the quantity at a specific price for a side
     */
    double
    get_level(SIDE side, double price) const
    {
        const auto& levels = side == SIDE::BUY ? bid_levels_ : ask_levels_;
        if (levels.find(price) == levels.end()) {
            return 0;
        }
        return levels.at(price);
    }

    double
    get_midprice() const
    {
        if (bids_.empty() || asks_.empty()) {
            return 0;
        }
        return (bids_.begin()->price + asks_.begin()->price) / 2;
    }

    std::pair<uint, uint>
    get_spread_nums() const
    {
        return {asks_.size(), bids_.size()};
    }

    std::pair<double, double>
    get_spread() const
    {
        if (asks_.empty() || bids_.empty()) [[unlikely]] {
            return {0, 0};
        }
        return {asks_.begin()->price, bids_.rbegin()->price};
    }

    bool
    can_match_orders() const
    {
        if (bids_.empty() || asks_.empty()) {
            return false;
        }
        return get_top_order(SIDE::BUY).can_match(get_top_order(SIDE::SELL));
    }

    void add_order(StoredOrder order);

    /**
     * @brief Expire all orders that were created tick-EXPIRATION_TIME ago
     * This should be called every tick
     */
    std::vector<StoredOrder> expire_orders(uint64_t tick);

    /**
     * @brief Modify the quantity of an order
     * Remove it if the quantity is now 0
     */
    void modify_order_quantity(uint64_t order_index, double delta);

    /**
     * @brief Remove an order from all data structures
     */
    StoredOrder remove_order(uint64_t order_id);

    /**
     * @brief Get the top order on a side
     */
    const StoredOrder& get_top_order(SIDE side) const;

private:
    const StoredOrder&
    get_order_(uint64_t order_id) const
    {
        assert(order_exists_(order_id));
        return orders_by_id_.at(order_id);
    }

    StoredOrder&
    get_order_(uint64_t order_id)
    {
        assert(order_exists_(order_id));
        return orders_by_id_.at(order_id);
    }

    bool
    order_exists_(uint64_t order_id) const
    {
        return orders_by_id_.find(order_id) != orders_by_id_.end();
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
};
} // namespace matching
} // namespace nutc
