#include "cycle_interface.hpp"

namespace nutc::matching {
void
MatchingCycleInterface::sort_by_timestamp(OrderVectorPair& orders)
{
    auto cmp_order_timestamps = []<typename OrderT>(
                                    const OrderT& lhs, const OrderT& rhs
                                ) { return lhs.time_received < rhs.time_received; };
    std::sort(orders.first.begin(), orders.first.end(), cmp_order_timestamps);
    std::sort(orders.second.begin(), orders.second.end(), cmp_order_timestamps);
}
} // namespace nutc::matching
