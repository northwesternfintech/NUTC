#pragma once

#include "exchange/logging.hpp"
#include "exchange/matching_cycle/base/base_cycle.hpp"
#include "exchange/orders/ticker_info.hpp"
#include "exchange/traders/trader_container.hpp"

#include <string>

std::string mo_to_string(const nutc::shared::limit_order& order);

namespace nutc::test {

class TestMatchingCycle : public exchange::BaseMatchingCycle {
public:
    std::optional<OrderVariant> last_order;

    TestMatchingCycle(
        std::vector<std::string> ticker_names, exchange::TraderContainer& traders,
        double order_fee = 0.0
    ) :
        exchange::BaseMatchingCycle{create_tickers(ticker_names, order_fee), traders}
    {}

    // Note: uses tick=0. If using something that relies on tick, it will not work
    template <typename OrderT>
    void
    wait_for_order(const OrderT& order)
    {
        log_i(testing, "Waiting for order {}", *glz::write_json(order));

        auto orders_are_same = [&]<typename LastOrder>(const LastOrder& last_order_v) {
            if constexpr (std::is_base_of_v<OrderT, LastOrder>) {
                return static_cast<const OrderT&>(last_order_v) == order;
            }
            return false;
        };

        while (!last_order.has_value() || !std::visit(orders_are_same, *last_order))
            on_tick(0);
        log_i(testing, "Expected order received. Continuing...");
    }

private:
    virtual std::vector<shared::match> match_orders_(std::vector<OrderVariant> orders
    ) override;

    exchange::TickerMapping
    create_tickers(const std::vector<std::string>& ticker_names, double order_fee);
};

} // namespace nutc::test
