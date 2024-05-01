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

    std::vector<stored_match> match_order(const stored_order& order);

    std::vector<stored_order>
    expire_old_orders(uint64_t new_tick)
    {
        auto orders = order_container_.expire_orders(new_tick - ORDER_EXPIRATION_TICKS);
        current_tick_ = new_tick;
        return orders;
    }

private:
    bool order_can_execute_(const stored_order& buyer, const stored_order& seller);

    static stored_match
    build_match(const stored_order& buyer, const stored_order& seller);

    std::vector<stored_match> attempt_matches_();
    void drop_order(uint64_t order_index);
};

} // namespace matching
} // namespace nutc
