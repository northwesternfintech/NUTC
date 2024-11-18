#pragma once

#include "common/types/decimal.hpp"
#include "exchange/matching_cycle/base/base_cycle.hpp"
#include "exchange/traders/trader_container.hpp"
#include "soft_equality.hpp"

#include <queue>

namespace nutc::test {

class TestMatchingCycle : public exchange::BaseMatchingCycle {
    std::queue<exchange::OrderVariant> incoming_orders_;

public:
    TestMatchingCycle(
        exchange::TraderContainer& traders, common::decimal_price order_fee = 0.0,
        common::decimal_quantity max_order_volume =
            std::numeric_limits<common::decimal_quantity>::max()
    ) : exchange::BaseMatchingCycle{{}, traders, order_fee, max_order_volume}
    {}

    // Note: uses tick=0. If using something that relies on tick, it will not work
    template <typename OrderT>
    const OrderT&
    wait_for_order(const OrderT& order, std::function<bool(const OrderT&, const OrderT&)> equality_function = soft_equality<OrderT>);

private:
    std::vector<exchange::tagged_match>
    match_orders_(std::vector<exchange::OrderVariant> orders) override;
};

} // namespace nutc::test
