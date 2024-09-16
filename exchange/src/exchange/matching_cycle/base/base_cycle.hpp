#pragma once
#include "exchange/matching_cycle/cycle_interface.hpp"
#include "exchange/orders/ticker_container.hpp"
#include "exchange/traders/trader_container.hpp"

namespace nutc::exchange {

/**
 * @brief Barebones matching cycle. Likely to be overridden for more logging
 */
class BaseMatchingCycle : public MatchingCycleInterface {
    TickerContainer tickers_;
    TraderContainer& traders_;
    common::decimal_price order_fee_;

public:
    // Require transfer of ownership
    BaseMatchingCycle(
        TickerContainer tickers, TraderContainer& traders, common::decimal_price order_fee
    ) : tickers_(std::move(tickers)), traders_(traders), order_fee_(order_fee)
    {}

protected:
    auto&
    get_tickers()
    {
        return tickers_;
    }

    TraderContainer&
    get_traders()
    {
        return traders_;
    }

    void before_cycle_(uint64_t) override;

    std::vector<OrderVariant> collect_orders(uint64_t) override;

    std::vector<common::match> match_orders_(std::vector<OrderVariant> orders) override;

    void handle_matches_(std::vector<common::match> matches) override;

    void
    post_cycle_(uint64_t) override
    {}
};

} // namespace nutc::exchange
