#include "test_cycle.hpp"

#include "exchange/orders/ticker_info.hpp"

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

exchange::TickerMapping
TestMatchingCycle::create_tickers(double order_fee)
{
    exchange::TickerMapping mappings{};
    for (auto ticker :
         {shared::Ticker::ETH, shared::Ticker::BTC, shared::Ticker::LTC}) {
        mappings.insert({
            ticker, {ticker, order_fee}
        });
    }
    return mappings;
}

} // namespace nutc::test
