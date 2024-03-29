#pragma once

#include "exchange/config.h"
#include "order_container.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"
#include "shared/util.hpp"

using MarketOrder = nutc::messages::MarketOrder;
using ObUpdate = nutc::messages::ObUpdate;
using Match = nutc::messages::Match;
using SIDE = nutc::messages::SIDE;

namespace nutc {
namespace matching {

class Engine {
    OrderContainer order_container_;
    uint64_t current_tick_ = 0;

public:
    const OrderContainer&
    get_order_container() const
    {
        return order_container_;
    }

    std::vector<StoredMatch> match_order(const MarketOrder& order);

    std::vector<StoredOrder>
    expire_old_orders(uint64_t new_tick)
    {
        auto orders = order_container_.expire_orders(new_tick - ORDER_EXPIRATION_TIME);
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
