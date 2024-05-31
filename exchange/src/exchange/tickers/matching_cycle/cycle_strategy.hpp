#pragma once

#include "exchange/tickers/engine/order_storage.hpp"

#include <vector>

namespace nutc {
namespace matching {

class MatchingCycle {
public:
    void
    on_tick(uint64_t new_tick)
    {
        before_cycle_(new_tick);
        auto orders = collect_orders(new_tick);
        auto matches = match_orders_(orders);
        handle_matches_(std::move(matches));
        post_cycle_(new_tick);
    }

private:
    // Logging, generate orders, etc
    virtual void before_cycle_(uint64_t new_tick) = 0;
    virtual std::vector<stored_order> collect_orders(uint64_t new_tick) = 0;
    virtual std::vector<stored_match> match_orders_(std::vector<stored_order> orders
    ) = 0;
    virtual void handle_matches_(std::vector<stored_match> matches) = 0;

    // Logging, send out updates, etc
    virtual void post_cycle_(uint64_t new_tick) = 0;

public:
    virtual ~MatchingCycle() {}
};
} // namespace matching
} // namespace nutc
