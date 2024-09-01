#include "test_cycle.hpp"

#include "common/messages_wrapper_to_exchange.hpp"
#include "common/util.hpp"

#include <glaze/glaze.hpp>
#include <hash_table7.hpp>

#include <utility>

namespace nutc::test {

std::vector<common::match>
TestMatchingCycle::match_orders_(std::vector<OrderVariant> orders)
{
    // TODO: FIX
    if (!orders.empty()) {
        auto& order = orders.back();
        last_order.emplace(order);
    }

    auto print_order = []<typename T>(const T& message) -> std::string {
        if constexpr (std::is_base_of_v<common::market_order, T>) {
            return *glz::write_json(static_cast<const common::market_order&>(message));
        }
        else if constexpr (std::is_base_of_v<common::limit_order, T>) {
            return *glz::write_json(static_cast<const common::limit_order&>(message));
        }
        else if constexpr (std::is_base_of_v<common::cancel_order, T>) {
            return *glz::write_json(static_cast<const common::cancel_order&>(message));
        }
        else {
            throw std::runtime_error("Unexpected message");
        }
    };

    for (auto& order_variant : orders) {
        std::string message = std::visit(print_order, order_variant);
        log_i(testing, "Received message from wrapper: {}", message);
    }

    return BaseMatchingCycle::match_orders_(std::move(orders));
}

// This is kinda bad practice. We shouldn't return an optional based on a template
// parameter, we should just return void or the object itself. However, I am lazy and
// this is a test function. Peace
template <typename OrderT>
std::optional<common::order_id_t>
TestMatchingCycle::wait_for_order(
    const OrderT& order,
    std::function<bool(const OrderT&, const OrderT&)> equality_function
)
{
    log_i(testing, "Waiting for order {}", *glz::write_json(order));

    auto orders_are_equal = [&]<typename LastOrder>(const LastOrder& last_order_v) {
        if constexpr (std::is_base_of_v<OrderT, LastOrder>) {
            return equality_function(order, static_cast<const OrderT&>(last_order_v));
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

template std::optional<common::order_id_t> TestMatchingCycle::
    wait_for_order<>(const common::limit_order&, std::function<bool(const common::limit_order&, const common::limit_order&)>);
template std::optional<common::order_id_t> TestMatchingCycle::
    wait_for_order<>(const common::market_order&, std::function<bool(const common::market_order&, const common::market_order&)>);
template std::optional<common::order_id_t> TestMatchingCycle::
    wait_for_order<>(const common::cancel_order&, std::function<bool(const common::cancel_order&, const common::cancel_order&)>);

} // namespace nutc::test
