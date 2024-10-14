#pragma once

#include "common/types/messages/messages_exchange_to_wrapper.hpp"
#include "exchange/orders/storage/order_storage.hpp"

#include <vector>

namespace nutc::exchange {

class MatchingCycleInterface {
public:
    void
    on_tick(uint64_t new_tick)
    {
        before_cycle_(new_tick);

        auto orders = collect_orders(new_tick);
        sort_by_timestamp(orders);

        auto matches = match_orders_(std::move(orders));
        handle_matches_(std::move(matches));

        post_cycle_(new_tick);
    }

    virtual ~MatchingCycleInterface() = default;

protected:
    virtual void before_cycle_(uint64_t new_tick) = 0;

    virtual std::vector<OrderVariant> collect_orders(uint64_t new_tick) = 0;

    virtual std::vector<tagged_match> match_orders_(std::vector<OrderVariant> orders
    ) = 0;

    virtual void handle_matches_(std::vector<tagged_match> matches) = 0;

    virtual void post_cycle_(uint64_t new_tick) = 0;

private:
    static void sort_by_timestamp(std::vector<OrderVariant>& orders);
};
} // namespace nutc::exchange
