#include "test_cycle.hpp"

#include "exchange/logging.hpp"
#include "exchange/orders/ticker_info.hpp"

#include <glaze/glaze.hpp>
#include <hash_table7.hpp>

#include <utility>

namespace nutc {
namespace test {

void
TestMatchingCycle::wait_for_order(const messages::limit_order& order)
{
    log_i(testing, "Waiting for order {}", glz::write_json(order));
    auto last = last_order == nullptr
                    ? nullptr
                    : std::make_unique<matching::stored_order>(*last_order);
    while (last == nullptr || *last != order) {
        on_tick(0);
        last = std::make_unique<matching::stored_order>(*last_order);
    }
    log_i(testing, "Expected order received. Continuing...");
}

std::vector<matching::stored_match>
TestMatchingCycle::match_orders_(std::vector<matching::stored_order> orders)
{
    if (!orders.empty()) {
        auto order = orders.back();
        log_i(
            testing, "Order received: {}", glz::write_json(messages::limit_order{order})
        );
        last_order =
            std::make_unique<matching::stored_order>(orders.at(orders.size() - 1));
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
