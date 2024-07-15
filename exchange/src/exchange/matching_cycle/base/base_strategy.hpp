#pragma once
#include "exchange/matching_cycle/cycle_strategy.hpp"
#include "exchange/orders/ticker_info.hpp"
#include <hash_table7.hpp>

#include <unordered_map>

namespace nutc {
namespace matching {
/**
 * @brief Barebones matching cycle. Likely to be overridden for more logging
 */
class BaseMatchingCycle : public MatchingCycle {
protected:
	emhash7::HashMap<util::Ticker, ticker_info> tickers_;
    std::vector<std::shared_ptr<traders::GenericTrader>>& traders_; // move elsewhere
    const uint64_t ORDER_EXPIRE_TICKS;

public:
    // Require transfer of ownership
    BaseMatchingCycle(
        emhash7::HashMap<util::Ticker, ticker_info> tickers,
        std::vector<std::shared_ptr<traders::GenericTrader>>& traders,
        uint64_t expire_ticks
    ) :
        tickers_(std::move(tickers)),
        traders_(traders), ORDER_EXPIRE_TICKS(expire_ticks)
    {}

protected:
    virtual void before_cycle_(uint64_t) override;

    virtual std::vector<stored_order> collect_orders(uint64_t) override;

    virtual std::vector<stored_match> match_orders_(std::vector<stored_order> orders
    ) override;

    virtual void handle_matches_(std::vector<stored_match> matches) override;

    virtual void post_cycle_(uint64_t) override;
};

} // namespace matching
} // namespace nutc
