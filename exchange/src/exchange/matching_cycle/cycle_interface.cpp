#include "cycle_interface.hpp"

namespace nutc::exchange {
void
MatchingCycleInterface::sort_by_timestamp(std::vector<OrderVariant>& orders)
{
    auto get_timestamp = [](const auto& order) { return order.timestamp; };

    auto cmp_order_timestamps =
        [&get_timestamp]<typename OrderT>(const OrderT& lhs, const OrderT& rhs) {
            return std::visit(get_timestamp, lhs) < std::visit(get_timestamp, rhs);
        };
    std::sort(orders.begin(), orders.end(), cmp_order_timestamps);
}
} // namespace nutc::exchange
