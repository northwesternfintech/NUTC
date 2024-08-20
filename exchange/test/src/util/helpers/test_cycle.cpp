#include "test_cycle.hpp"

#include "exchange/logging.hpp"
#include "exchange/orders/storage/order_storage.hpp"
#include "exchange/orders/ticker_info.hpp"

#include <glaze/glaze.hpp>
#include <hash_table7.hpp>

#include <utility>

namespace nutc {
namespace test {

void
TestMatchingCycle::wait_for_order(const messages::limit_order& order)
{
    log_i(testing, "Waiting for order {}", *glz::write_json(order));
    while (!last_order.has_value() || last_order.value() != order) {
        on_tick(0);
    }
    log_i(testing, "Expected order received. Continuing...");
}

std::vector<matching::stored_match>
TestMatchingCycle::match_orders_(OrderVectorPair orders)
{
    // TODO: FIX
    if (!orders.first.empty()) {
        auto& order = orders.first.back();
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
