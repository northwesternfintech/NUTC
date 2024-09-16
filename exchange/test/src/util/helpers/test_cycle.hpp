#pragma once

#include "common/util.hpp"
#include "exchange/logging.hpp"
#include "exchange/matching_cycle/base/base_cycle.hpp"
#include "exchange/traders/trader_container.hpp"
#include "soft_equality.hpp"

#include <string>

std::string mo_to_string(const nutc::common::limit_order& order);

namespace nutc::test {

class TestMatchingCycle : public exchange::BaseMatchingCycle {
    std::optional<exchange::OrderVariant> last_order;

public:
    TestMatchingCycle(exchange::TraderContainer& traders, common::decimal_price order_fee = 0.0) :
        exchange::BaseMatchingCycle{{}, traders, order_fee}
    {}

    // Note: uses tick=0. If using something that relies on tick, it will not work
    template <typename OrderT>
    std::optional<common::order_id_t>
    wait_for_order(const OrderT& order, std::function<bool(const OrderT&, const OrderT&)> equality_function = soft_equality<OrderT>);

private:
    std::vector<common::match> match_orders_(std::vector<exchange::OrderVariant> orders
    ) override;
};

} // namespace nutc::test
