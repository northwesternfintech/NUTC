#pragma once
#include "exchange/tickers/engine/order_storage.hpp"
#include "exchange/traders/trader_types/bot_trader.hpp"
#include "exchange/traders/trader_types/generic_trader.hpp"

#include <sys/types.h>

#include <vector>

namespace nutc {
namespace bots {

// TODO for hardening: if price gets close to 0, quantity will get very high because we
// divide by price. Maybe something to think about?
class MarketMakerBot : public BotTrader {
public:
    MarketMakerBot(std::string ticker, double interest_limit) :
        BotTrader(std::move(ticker), interest_limit)
    {}

    bool constexpr can_leverage() const override { return true; }

    std::vector<matching::StoredOrder>
    take_action(double new_theo, uint64_t current_tick);

    [[nodiscard]] bool
    is_active() const override
    {
        return true;
    }

private:
    [[nodiscard]] double
    compute_net_exposure_() const
    {
        return (get_long_interest() - get_short_interest());
    }

    double
    compute_capital_tolerance_()
    {
        return (1 - get_capital_utilization()) * (get_interest_limit() / 3);
    }

    static constexpr double avg_level_price(double new_theo);

    std::shared_ptr<manager::GenericTrader> get_self_pointer_();

    void process_order_match(
        const std::string& ticker, util::Side side, double price, double quantity
    ) override;

    void process_order_expiration(
        const std::string& ticker, util::Side side, double price, double quantity
    ) override;
};

} // namespace bots
} // namespace nutc
