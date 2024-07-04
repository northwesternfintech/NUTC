#pragma once

#include "exchange/orders/storage/level_update_generator.hpp"
#include "exchange/orders/storage/order_storage.hpp"

#include <cassert>

#include <queue>
#include <vector>

namespace nutc {
namespace matching {

class OrderBook {
    std::shared_ptr<LevelUpdateGenerator> level_update_generator_;

    // both map/sort price, order_index
    std::map<decimal_price, std::queue<stored_order>> bids_;
    std::map<decimal_price, std::queue<stored_order>> asks_;

    std::unordered_map<uint64_t, std::reference_wrapper<stored_order>> order_map_;
	std::vector<uint64_t> ioc_order_ids_;

public:
    // Default constructor for testing the orderbook function without the need of a
    // LevelUpdateGenerator
    explicit OrderBook() : level_update_generator_(nullptr) {}

    explicit OrderBook(std::shared_ptr<LevelUpdateGenerator> level_update_generator) :
        level_update_generator_(level_update_generator)
    {}

    decimal_price get_midprice() const;

    bool can_match_orders();

    void add_order(stored_order order);

    std::vector<stored_order> remove_ioc_orders();

    /**
     * @brief Get the top order on a side
     */
    stored_order& get_top_order(util::Side side);

    stored_order& mark_order_removed(stored_order& order);
    stored_order& mark_order_removed(uint64_t order_id);

    void change_quantity(stored_order& order, double quantity_delta);

private:
    void clean_tree(util::Side side);
    stored_order pop_from_queue(util::Side side, double price);
    void modify_level_(util::Side side, decimal_price price, double quantity);
};
} // namespace matching
} // namespace nutc
