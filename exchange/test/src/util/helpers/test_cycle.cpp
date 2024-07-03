#include "test_cycle.hpp"

#include "exchange/logging.hpp"
#include "shared/ticker.hpp"

#include <glaze/glaze.hpp>

#include <tuple>
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
            testing, "Order received: {} {} {}", std::string{order.ticker}, double{order.price},
            order.quantity
        );
        last_order =
            std::make_unique<matching::stored_order>(orders.at(orders.size() - 1));
    }

    return BaseMatchingCycle::match_orders_(std::move(orders));
}

std::unordered_map<util::Ticker, matching::ticker_info>
TestMatchingCycle::create_tickers(
    const std::vector<std::string>& ticker_names, double order_fee
)
{
    std::unordered_map<util::Ticker, matching::ticker_info> mappings;
    for (const auto& ticker : ticker_names) {
        mappings.emplace(
            std::piecewise_construct, std::forward_as_tuple(ticker.c_str()),
            std::forward_as_tuple(ticker.c_str(), order_fee)
        );
    }
    return mappings;
}

} // namespace test
} // namespace nutc
