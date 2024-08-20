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
        auto matches = match_orders_(std::move(orders));
        handle_matches_(std::move(matches));
        post_cycle_(new_tick);
    }

protected:
    using TaggedOrderVariant = std::variant<tagged_limit_order, tagged_market_order>;

    virtual void before_cycle_(uint64_t new_tick) = 0;

    virtual std::vector<TaggedOrderVariant> collect_orders(uint64_t new_tick) = 0;

    virtual std::vector<stored_match>
    match_orders_(std::vector<TaggedOrderVariant> orders) = 0;

    virtual void handle_matches_(std::vector<stored_match> matches) = 0;

    virtual void post_cycle_(uint64_t new_tick) = 0;

public:
    virtual ~MatchingCycleInterface() = default;
};
} // namespace matching
} // namespace nutc
