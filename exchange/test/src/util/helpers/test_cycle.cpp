#include "test_cycle.hpp"

#include "common/logging/logging.hpp"
#include "common/types/messages/messages_wrapper_to_exchange.hpp"
#include "common/types/visitor.hpp"

#include <glaze/json/write.hpp>
#include <hash_table7.hpp>

#include <queue>
#include <utility>

namespace nutc::test {

namespace {

auto
get_base_order(const auto& order)
{
    return common::make_visitor(
        [](const exchange::tagged_limit_order& order) {
            return static_cast<const common::limit_order&>(order);
        },
        [](const exchange::tagged_market_order& order) {
            return static_cast<const common::market_order&>(order);
        },
        [](const common::cancel_order& order) {
            return static_cast<const common::cancel_order&>(order);
        }
    )(order);
}

} // namespace

std::vector<exchange::tagged_match>
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

// TODO: this doesn't work correctly when we have orders of the same type that need
// to be processed. Rework this entire thing later
template <typename OrderT>
const OrderT&
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
    auto try_cast = [](const auto& order) -> const OrderT& {
        if constexpr (std::is_constructible_v<const OrderT&, decltype(order)>) {
            return static_cast<const OrderT&>(order);
        }
        throw std::runtime_error("Unexpected order type - should be unreachable");
    };
    return std::visit(try_cast, incoming_orders_.front()); // NOLINT
}

template const common::limit_order& TestMatchingCycle::
    wait_for_order<>(const common::limit_order&, std::function<bool(const common::limit_order&, const common::limit_order&)>);
template const common::market_order& TestMatchingCycle::
    wait_for_order<>(const common::market_order&, std::function<bool(const common::market_order&, const common::market_order&)>);
template const common::cancel_order& TestMatchingCycle::
    wait_for_order<>(const common::cancel_order&, std::function<bool(const common::cancel_order&, const common::cancel_order&)>);

} // namespace nutc::test
