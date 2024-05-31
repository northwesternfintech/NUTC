#pragma once

#include "exchange/tickers/ticker.hpp"
#include "exchange/tickers/matching_cycle/base/base_strategy.hpp"

#include <memory>
#include <tuple>
#include <utility>

namespace nutc {
namespace test {

class TestMatchingCycle : public matching::BaseMatchingCycle {
public:
    matching::stored_order last_order;

    TestMatchingCycle(
        std::vector<std::string> ticker_names,
        std::vector<std::shared_ptr<traders::GenericTrader>> traders,
        double order_fee = 0.0,
        uint64_t order_expire_ticks = std::numeric_limits<uint64_t>::max()
    ) :
        matching::BaseMatchingCycle{
            create_tickers(ticker_names, order_fee), traders, order_expire_ticks
        }
    {}

    void
    wait_for_order(const messages::market_order& order)
    {
        messages::market_order last = last_order;
        while (last != order) {
            on_tick(0);
            last = last_order;
        }
    }

private:
    virtual std::vector<matching::stored_match>
    match_orders_(std::vector<matching::stored_order> orders) override
    {
        if (!orders.empty())
            last_order = orders.at(orders.size() - 1);

        return BaseMatchingCycle::match_orders_(std::move(orders));
    }

    std::unordered_map<std::string, matching::ticker_info>
    create_tickers(const std::vector<std::string>& ticker_names, double order_fee)
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
};

} // namespace test
} // namespace nutc
