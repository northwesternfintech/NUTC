#pragma once

#include "exchange/tickers/matching_cycle/base/base_strategy.hpp"
#include "exchange/tickers/ticker.hpp"

#include <memory>
#include <string>

std::string mo_to_string(const nutc::messages::market_order& order);

namespace nutc {
namespace test {

class TestMatchingCycle : public matching::BaseMatchingCycle {
public:
    std::unique_ptr<matching::stored_order> last_order;

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

    // Note: uses tick=0. If using something that relies on tick, it will not work
    void wait_for_order(const messages::market_order& order);

private:
    virtual std::vector<matching::stored_match>
    match_orders_(std::vector<matching::stored_order> orders) override;

    std::unordered_map<std::string, matching::ticker_info>
    create_tickers(const std::vector<std::string>& ticker_names, double order_fee);
};

} // namespace test
} // namespace nutc
