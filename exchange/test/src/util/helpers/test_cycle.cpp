#include "test_cycle.hpp"

#include "common/logging/logging.hpp"
#include "common/types/messages/messages_wrapper_to_exchange.hpp"
#include "common/types/messages/glz_messages_wrapper_to_exchange.hpp"
#include "common/util.hpp"

#include <glaze/glaze.hpp>
#include <hash_table7.hpp>

#include <queue>
#include <utility>

namespace nutc::test {

namespace {
template <typename OrderT>
auto
get_base_order(const OrderT& order)
{
    if constexpr (std::is_same_v<exchange::tagged_limit_order, OrderT>) {
        return static_cast<const common::limit_order&>(order);
    }
    else if constexpr (std::is_same_v<exchange::tagged_market_order, OrderT>) {
        return static_cast<const common::market_order&>(order);
    }
    else if constexpr (std::is_same_v<common::cancel_order, OrderT>) {
        return static_cast<const common::cancel_order&>(order);
    }
    else {
        throw std::runtime_error("Unexpected order");
    }
}
} // namespace

std::vector<common::match>
TestMatchingCycle::match_orders_(std::vector<exchange::OrderVariant> orders)
{
    for (auto& order : orders) {
        incoming_orders_.push(order);
    }

    auto print_order = []<typename T>(const T& message) -> std::string {
        return *glz::write_json(get_base_order(message));
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
//
// Also todo: this doesn't work correctly when we have orders of the same type that need
// to be processed. Rework this entire thing later
template <typename OrderT>
std::optional<common::order_id_t>
TestMatchingCycle::wait_for_order(
    const OrderT& order,
    std::function<bool(const OrderT&, const OrderT&)> equality_function
)
{
    log_i(testing, "Waiting for order {}", *glz::write_json(order));

    auto orders_are_equal = [&](const auto& last_order) {
        const auto& base_order = get_base_order(last_order);
        if constexpr (std::is_same_v<
                          OrderT, std::remove_cvref_t<decltype(base_order)>>) {
            return equality_function(base_order, order);
        }
        return false;
    };

    while (incoming_orders_.empty()
           || !std::visit(orders_are_equal, incoming_orders_.front())) {
        if (!incoming_orders_.empty()) {
            incoming_orders_.pop();
        }
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
    return std::visit(get_order_id, incoming_orders_.front());
}

template std::optional<common::order_id_t> TestMatchingCycle::
    wait_for_order<>(const common::limit_order&, std::function<bool(const common::limit_order&, const common::limit_order&)>);
template std::optional<common::order_id_t> TestMatchingCycle::
    wait_for_order<>(const common::market_order&, std::function<bool(const common::market_order&, const common::market_order&)>);
template std::optional<common::order_id_t> TestMatchingCycle::
    wait_for_order<>(const common::cancel_order&, std::function<bool(const common::cancel_order&, const common::cancel_order&)>);

} // namespace nutc::test
