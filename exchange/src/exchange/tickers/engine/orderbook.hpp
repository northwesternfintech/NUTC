#pragma once

#include "level_update_generator.hpp"
#include "order_storage.hpp"
#include "shared/util.hpp"

#include <cassert>

#include <set>
#include <unordered_map>
#include <vector>

namespace nutc {
namespace matching {

class OrderBook {
    std::shared_ptr<LevelUpdateGenerator> level_update_generator_;

    // both map/sort price, order_index
    std::set<order_index, bid_comparator> bids_;
    std::set<order_index, ask_comparator> asks_;

    // order index -> order
    std::unordered_map<uint64_t, stored_order> orders_by_id_;

    // tick -> queue of order ids
    std::unordered_map<uint64_t, std::vector<uint64_t>> orders_by_tick_;

    std::unordered_map<decimal_price, double> bid_levels_;
    std::unordered_map<decimal_price, double> ask_levels_;

public:
    // Default constructor for testing the orderbook function without the need of a
    // LevelUpdateGenerator
    explicit OrderBook() : level_update_generator_(nullptr) {}

    explicit OrderBook(std::shared_ptr<LevelUpdateGenerator> level_update_generator) :
        level_update_generator_(level_update_generator)
    {}

    /**
     * @brief Get the quantity at a specific price for a side
     */
    double get_level(util::Side side, double price) const;

    double get_midprice() const;

    bool can_match_orders() const;

    void add_order(stored_order order);

    /**
     * @brief Expire all orders that were created tick-EXPIRATION_TIME ago
     * This should be called every tick
     */
    std::vector<stored_order> expire_orders(uint64_t tick);

    /**
     * @brief Modify the quantity of an order
     * Remove it if the quantity is now 0
     */
    void modify_order_quantity(uint64_t order_index, double delta);

    /**
     * @brief Remove an order from all data structures
     */
    stored_order remove_order(uint64_t order_id);

    /**
     * @brief Get the top order on a side
     */
    const stored_order& get_top_order(util::Side side) const;

private:
    const stored_order&
    get_order_(uint64_t order_id) const
    {
        assert(order_exists_(order_id));
        return orders_by_id_.at(order_id);
    }

    stored_order&
    get_order_(uint64_t order_id)
    {
        assert(order_exists_(order_id));
        return orders_by_id_.at(order_id);
    }

    bool
    order_exists_(uint64_t order_id) const
    {
        return orders_by_id_.contains(order_id);
    }

    void modify_level_(util::Side side, double price, double qualtity);
};
} // namespace matching
} // namespace nutc
