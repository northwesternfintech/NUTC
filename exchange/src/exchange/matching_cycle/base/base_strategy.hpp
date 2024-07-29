#pragma once
#include "exchange/matching_cycle/cycle_strategy.hpp"
#include "exchange/orders/ticker_info.hpp"
#include "exchange/traders/trader_container.hpp"

namespace nutc {
namespace matching {

/**
 * @brief Barebones matching cycle. Likely to be overridden for more logging
 */
class BaseMatchingCycle : public MatchingCycle {
    TickerMapping tickers_;
    traders::TraderContainer& trader_container;
    const uint64_t ORDER_EXPIRE_TICKS;

public:
    // Require transfer of ownership
    BaseMatchingCycle(
        TickerMapping tickers, traders::TraderContainer& traders, uint64_t expire_ticks
    ) :
        tickers_(std::move(tickers)), trader_container(traders),
        ORDER_EXPIRE_TICKS(expire_ticks)
    {}

protected:
    auto&
    get_tickers()
    {
        return tickers_;
    }

    auto&
    get_trader_container()
    {
        return trader_container;
    }

    virtual void before_cycle_(uint64_t) override;

    virtual std::vector<stored_order> collect_orders(uint64_t) override;

    virtual std::vector<stored_match> match_orders_(std::vector<stored_order> orders
    ) override;

    virtual void handle_matches_(std::vector<stored_match> matches) override;

    virtual void post_cycle_(uint64_t) override;
};

} // namespace matching
} // namespace nutc
