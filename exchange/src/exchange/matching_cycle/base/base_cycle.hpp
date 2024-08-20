#pragma once
#include "exchange/matching_cycle/cycle_interface.hpp"
#include "exchange/orders/ticker_info.hpp"
#include "exchange/traders/trader_container.hpp"

namespace nutc {
namespace matching {

/**
 * @brief Barebones matching cycle. Likely to be overridden for more logging
 */
class BaseMatchingCycle : public MatchingCycleInterface {
    TickerMapping tickers_;
    traders::TraderContainer& traders_;

public:
    // Require transfer of ownership
    BaseMatchingCycle(TickerMapping tickers, traders::TraderContainer& traders) :
        tickers_(std::move(tickers)), traders_(traders)
    {}

protected:
    auto&
    get_tickers()
    {
        return tickers_;
    }

    traders::TraderContainer&
    get_traders()
    {
        return traders_;
    }

    virtual void before_cycle_(uint64_t) override;

    virtual std::vector<OrderVariant> collect_orders(uint64_t) override;

    virtual std::vector<stored_match> match_orders_(std::vector<OrderVariant> orders
    ) override;

    virtual void handle_matches_(std::vector<stored_match> matches) override;

    virtual void post_cycle_(uint64_t) override;
};

} // namespace matching
} // namespace nutc
