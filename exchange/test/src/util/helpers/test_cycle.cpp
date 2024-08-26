#include "test_cycle.hpp"

#include <glaze/glaze.hpp>
#include <hash_table7.hpp>

#include <utility>

namespace nutc::test {

std::vector<shared::match>
TestMatchingCycle::match_orders_(std::vector<OrderVariant> orders)
{
    // TODO: FIX
    if (!orders.empty()) {
        auto& order = orders.back();
        last_order.emplace(order);
    }

    return BaseMatchingCycle::match_orders_(std::move(orders));
}

} // namespace nutc::test
