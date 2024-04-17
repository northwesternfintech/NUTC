#pragma once

#include "order_container.hpp"

namespace nutc {
namespace matching {

class Engine {
    const size_t ORDER_EXPIRATION_TICKS = 0;
    OrderContainer order_container_;
    uint64_t current_tick_ = 0;

public:
    explicit Engine(size_t order_expiration_ticks) :
        ORDER_EXPIRATION_TICKS(order_expiration_ticks)
    {}

    const OrderContainer&
    get_order_container() const
    {
        return order_container_;
    }

    std::vector<StoredMatch> match_order(const StoredOrder& order);

    std::vector<StoredOrder>
    expire_old_orders(uint64_t new_tick)
    {
        auto orders = order_container_.expire_orders(new_tick - ORDER_EXPIRATION_TICKS);
        current_tick_ = new_tick;
        return orders;
    }

private:
    bool order_can_execute_(const StoredOrder& buyer, const StoredOrder& seller);

    static StoredMatch build_match(const StoredOrder& buyer, const StoredOrder& seller);

    std::vector<StoredMatch> attempt_matches_();
};

} // namespace matching
} // namespace nutc
