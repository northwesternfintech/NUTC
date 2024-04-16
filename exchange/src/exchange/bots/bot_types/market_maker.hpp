#pragma once
#include "exchange/tickers/engine/order_storage.hpp"
#include "exchange/traders/trader_types/bot_trader.hpp"
#include "exchange/traders/trader_types/generic_trader.hpp"

#include <sys/types.h>

#include <array>
#include <map>
#include <vector>

namespace nutc {
namespace bots {

namespace {
struct price_level {
    const double PRICE_DELTA;
    const double QUANTITY_FACTOR;

    consteval price_level(double price_delta, double quantity_factor) :
        PRICE_DELTA(price_delta), QUANTITY_FACTOR(quantity_factor)
    {}
};
} // namespace

// TODO for hardening: if price gets close to 0, quantity will get very high because we
// divide by price. Maybe something to think about?
class MarketMakerBot : public BotTrader {
    static constexpr double BASE_SPREAD = 0.16;
    static constexpr uint8_t LEVELS = 3;
    static constexpr std::array<price_level, LEVELS> BID_LEVELS{
        price_level{-BASE_SPREAD - .10, 1.0 / 12},
        price_level{-BASE_SPREAD - .05, 1.0 / 6 },
        price_level{-BASE_SPREAD - .00, 1.0 / 4 }
    };
    static constexpr std::array<price_level, LEVELS> ASK_LEVELS{
        price_level{BASE_SPREAD + .00, 1.0 / 4 },
        price_level{BASE_SPREAD + .05, 1.0 / 6 },
        price_level{BASE_SPREAD + .10, 1.0 / 12}
    };

public:
    MarketMakerBot(MarketMakerBot&& other) = default;

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

    double avg_level_price(double new_theo);
    std::shared_ptr<manager::GenericTrader> get_self_pointer();

    void process_order_match(
        const std::string& ticker, messages::SIDE side, double price, double quantity
    ) override;

    void process_order_expiration(
        const std::string& ticker, messages::SIDE side, double price, double quantity
    ) override;
};

} // namespace bots
} // namespace nutc
