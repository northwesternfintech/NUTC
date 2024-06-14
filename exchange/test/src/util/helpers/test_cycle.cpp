#include "test_cycle.hpp"

#include "exchange/logging.hpp"
#include <glaze/glaze.hpp>

#include <tuple>
#include <utility>

namespace nutc {
namespace test {

void
TestMatchingCycle::wait_for_order(const messages::market_order& order)
{
    log_i(testing, "Waiting for order {}", glz::write_json(order));
    messages::market_order last = last_order;
    while (last != order) {
        on_tick(0);
        last = last_order;
    }
    log_i(testing, "Order received. Continuing...");
}

std::vector<matching::stored_match>
TestMatchingCycle::match_orders_(std::vector<matching::stored_order> orders)
{
    if (!orders.empty())
        last_order = orders.at(orders.size() - 1);

    return BaseMatchingCycle::match_orders_(std::move(orders));
}

std::unordered_map<std::string, matching::ticker_info>
TestMatchingCycle::create_tickers(
    const std::vector<std::string>& ticker_names, double order_fee
)
{
    std::unordered_map<std::string, matching::ticker_info> mappings;
    for (const auto& ticker : ticker_names) {
        mappings.emplace(
            std::piecewise_construct, std::forward_as_tuple(ticker),
            std::forward_as_tuple(ticker, order_fee)
        );
    }
    return mappings;
}

} // namespace test
} // namespace nutc
