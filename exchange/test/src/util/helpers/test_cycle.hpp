#pragma once

#include "common/util.hpp"
#include "exchange/logging.hpp"
#include "exchange/matching_cycle/base/base_cycle.hpp"
#include "exchange/traders/trader_container.hpp"

#include <string>

std::string mo_to_string(const nutc::common::limit_order& order);

namespace nutc::test {

class TestMatchingCycle : public exchange::BaseMatchingCycle {
public:
    std::optional<OrderVariant> last_order;

    TestMatchingCycle(exchange::TraderContainer& traders, double order_fee = 0.0) :
        exchange::BaseMatchingCycle{{order_fee}, traders}
    {}

    // Note: uses tick=0. If using something that relies on tick, it will not work
    // TODO: take in equality function
    template <typename OrderT>
    std::optional<common::order_id_t>
    wait_for_order(const OrderT& order)
    {
        log_i(testing, "Waiting for order {}", *glz::write_json(order));

        auto orders_are_equal = [&]<typename LastOrder>(const LastOrder& last_order_v) {
            if constexpr (std::is_base_of_v<OrderT, LastOrder>) {
                return static_cast<const OrderT&>(last_order_v) == order;
            }
            return false;
        };

        while (!last_order.has_value() || !std::visit(orders_are_equal, *last_order)) {
            on_tick(0);
        }
        log_i(testing, "Expected order received. Continuing...");
        auto get_order_id = []<typename SecondOrderT>(const SecondOrderT& order
                            ) -> std::optional<common::order_id_t> {
            if constexpr (std::is_same_v<exchange::tagged_limit_order, SecondOrderT>) {
                return order.order_id;
            }
            else {
                return std::nullopt;
            }
        };
        return std::visit(get_order_id, *last_order);
    }

private:
    std::vector<common::match> match_orders_(std::vector<OrderVariant> orders) override;
};

} // namespace nutc::test
