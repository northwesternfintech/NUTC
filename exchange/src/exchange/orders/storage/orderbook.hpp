#pragma once

#include "exchange/orders/storage/level_update_generator.hpp"
#include "exchange/orders/storage/order_storage.hpp"
#include "shared/ticker.hpp"

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

    std::vector<double> bid_levels_{1000};
    std::vector<double> ask_levels_{1000};

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
    double get_level(util::Side side, decimal_price price) const;

    decimal_price get_midprice() const;

    bool can_match_orders();

    void add_order(stored_order order);

    /**
     * @brief Expire all orders that were created tick-EXPIRATION_TIME ago
     * This should be called every tick
     */
    std::vector<stored_order> expire_orders(uint64_t tick);

    /**
     * @brief Get the top order on a side
     */
    stored_order& get_top_order(util::Side side);

    void modify_quantity(const stored_order& order, double quantity);
    void modify_level_(util::Side side, decimal_price price, double quantity);

private:
    void clean_tree(util::Side side);
};
} // namespace matching
} // namespace nutc
