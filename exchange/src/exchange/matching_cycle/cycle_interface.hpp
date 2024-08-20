#pragma once

#include "exchange/orders/storage/order_storage.hpp"

#include <vector>

namespace nutc {
namespace matching {

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
    using OrderVectorPair =
        std::pair<std::vector<tagged_limit_order>, std::vector<tagged_market_order>>;

    virtual void before_cycle_(uint64_t new_tick) = 0;

    virtual OrderVectorPair collect_orders(uint64_t new_tick) = 0;

    virtual std::vector<stored_match> match_orders_(OrderVectorPair orders) = 0;

    virtual void handle_matches_(std::vector<stored_match> matches) = 0;

    virtual void post_cycle_(uint64_t new_tick) = 0;

private:
    static void sort_by_timestamp(OrderVectorPair& orders);
};
} // namespace matching
} // namespace nutc
