#pragma once
#include "exchange/matching_cycle/cycle_interface.hpp"
#include "exchange/orders/ticker_info.hpp"
#include "exchange/traders/trader_container.hpp"

namespace nutc::exchange {

/**
 * @brief Barebones matching cycle. Likely to be overridden for more logging
 */
class BaseMatchingCycle : public MatchingCycleInterface {
    TickerMapping tickers_;
    TraderContainer& traders_;

public:
    // Require transfer of ownership
    BaseMatchingCycle(TickerMapping tickers, TraderContainer& traders) :
        tickers_(std::move(tickers)), traders_(traders)
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

    std::vector<shared::match> match_orders_(std::vector<OrderVariant> orders) override;

    void handle_matches_(std::vector<shared::match> matches) override;

    void post_cycle_(uint64_t) override;
};

} // namespace nutc::exchange
