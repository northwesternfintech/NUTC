#include "test_cycle.hpp"

#include "exchange/orders/ticker_info.hpp"

#include <glaze/glaze.hpp>
#include <hash_table7.hpp>

#include <utility>

namespace nutc {
namespace test {

std::vector<messages::match>
TestMatchingCycle::match_orders_(std::vector<OrderVariant> orders)
{
    // TODO: FIX
    if (!orders.empty()) {
        auto& order = orders.back();
        last_order.emplace(order);
    }

    return BaseMatchingCycle::match_orders_(std::move(orders));
}

matching::TickerMapping
TestMatchingCycle::create_tickers(
    const std::vector<std::string>& ticker_names, double order_fee
)
{
    matching::TickerMapping mappings;
    for (const auto& ticker : ticker_names) {
        util::Ticker t = ticker.c_str();
        mappings.insert({t, {order_fee}});
    }
    return mappings;
}

} // namespace test
} // namespace nutc
