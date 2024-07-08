#pragma once

#include "exchange/orders/level_tracking/level_update_generator.hpp"
#include "exchange/orders/orderbook/limit_orderbook.hpp"

#include <vector>

namespace nutc {
namespace matching {

class TrackedOrderBook : public LimitOrderBook {
    std::shared_ptr<LevelUpdateGenerator> level_update_generator_;
    std::vector<uint64_t> ioc_order_ids_;

    // Invariant: any order in order_map has a corresponding reference in the queues
    // In other words, when we remove from the queue, we remove from order map as well
    std::unordered_map<uint64_t, std::reference_wrapper<stored_order>> order_map_;

public:
    TrackedOrderBook() : level_update_generator_(nullptr) {}

    TrackedOrderBook(std::shared_ptr<LevelUpdateGenerator> update_generator) :
        level_update_generator_(std::move(update_generator))
    {}

    stored_order& add_order(stored_order order) override;
    void mark_order_removed(stored_order& order) override;
    void change_quantity(stored_order& order, double quantity_delta) override;

    std::vector<stored_order> remove_ioc_orders();

    stored_order& mark_order_removed(uint64_t order_id);
    bool contains_order(uint64_t order_id) const;

private:
    void modify_level_(util::Side side, decimal_price price, double delta);
};

} // namespace matching
} // namespace nutc
