#pragma once
#include "exchange/tickers/matching_cycle/cycle_strategy.hpp"
#include "exchange/tickers/ticker.hpp"

#include <unordered_map>

namespace nutc {
namespace matching {
/**
 * @brief Barebones matching cycle. Likely to be overridden for more logging
 */
class BaseMatchingCycle : public MatchingCycle {
protected:
    std::unordered_map<std::string, ticker_info> tickers_;
    std::vector<std::shared_ptr<traders::GenericTrader>> traders_; // move elsewhere
    const uint64_t ORDER_EXPIRE_TICKS;

public:
    // Require transfer of ownership
    BaseMatchingCycle(
        std::unordered_map<std::string, ticker_info> tickers,
        std::vector<std::shared_ptr<traders::GenericTrader>> traders,
        uint64_t expire_ticks
    ) :
        tickers_(std::move(tickers)), traders_(std::move(traders)),
        ORDER_EXPIRE_TICKS(expire_ticks)
    {}

protected:
    virtual void before_cycle_(uint64_t new_tick) override;

    virtual std::vector<stored_order> collect_orders(uint64_t new_tick) override;

    virtual std::vector<stored_match> match_orders_(std::vector<stored_order> orders
    ) override;

    virtual void handle_matches_(std::vector<stored_match> matches) override;

    virtual void post_cycle_(uint64_t) override;

    void
    expire_old_orders_(OrderBook& orderbook, uint64_t new_tick)
    {
		if(ORDER_EXPIRE_TICKS > new_tick) return;
        orderbook.expire_orders(new_tick - ORDER_EXPIRE_TICKS);
    }

    void
    generate_bot_orders_(bots::BotContainer& bot_container, const OrderBook& orderbook);
};

} // namespace matching
} // namespace nutc
