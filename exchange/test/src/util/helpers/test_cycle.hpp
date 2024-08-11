#pragma once

#include "exchange/matching_cycle/base/base_cycle.hpp"
#include "exchange/orders/ticker_info.hpp"
#include "exchange/traders/trader_container.hpp"

#include <memory>
#include <string>

std::string mo_to_string(const nutc::messages::limit_order& order);

namespace nutc {
namespace test {

class TestMatchingCycle : public matching::BaseMatchingCycle {
public:
    std::unique_ptr<matching::stored_order> last_order;

    TestMatchingCycle(
        std::vector<std::string> ticker_names, traders::TraderContainer& traders,
        double order_fee = 0.0
    ) : matching::BaseMatchingCycle{create_tickers(ticker_names, order_fee), traders}
    {}

    // Note: uses tick=0. If using something that relies on tick, it will not work
    void wait_for_order(const messages::limit_order& order);

private:
    virtual std::vector<matching::stored_match>
    match_orders_(std::vector<matching::stored_order> orders) override;

    matching::TickerMapping
    create_tickers(const std::vector<std::string>& ticker_names, double order_fee);
};

} // namespace test
} // namespace nutc
