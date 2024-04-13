#pragma once
#include "exchange/tickers/engine/order_storage.hpp"
#include "exchange/traders/trader_types/bot_trader.hpp"
#include "exchange/traders/trader_types/generic_trader.hpp"

#include <sys/types.h>

#include <cstdint>

#include <array>
#include <memory>
#include <vector>

namespace nutc {
namespace bots {

// TODO for hardening: if price gets close to 0, quantity will get very high because we
// divide by price. Maybe something to think about?
class MarketMakerBot : public BotTrader {
public:
    MarketMakerBot(MarketMakerBot&& other) = default;

    MarketMakerBot(std::string ticker, double interest_limit) :
        BotTrader(std::move(ticker), interest_limit)
    {}

    static constexpr double BASE_SPREAD = 0.16;

    double
    get_capital() const override
    {
        return std::numeric_limits<double>::max();
    }

    bool
    can_leverage() const override
    {
        return true;
    }

    std::vector<matching::StoredOrder>
    take_action(double new_theo, uint64_t current_tick)
    {
        double long_interest = get_long_interest();
        double short_interest = get_short_interest();

        double interest_limit = get_interest_limit();

        static constexpr const uint8_t LEVELS = 6;

        static constexpr const std::array<double, LEVELS> quantities = {
            1.0 / 12, 1.0 / 6, 1.0 / 4, 1.0 / 4, 1.0 / 6, 1.0 / 12
        };

        std::array<double, LEVELS> prices = {
            new_theo - BASE_SPREAD - .10, new_theo - BASE_SPREAD - .05,
            new_theo - BASE_SPREAD,       new_theo + BASE_SPREAD,
            new_theo + BASE_SPREAD + .05, new_theo + BASE_SPREAD * .10,
        };

        double capital_tolerance = compute_capital_tolerance_();
        double lean =
            -1 * ((long_interest - short_interest) / (long_interest + short_interest))
            * interest_limit * 2.7; // NOLINT(*)

        if (true || long_interest + short_interest == 0)
            lean = 0;
        for (auto& price : prices) {
            price += lean;
        }

        double avg_price = 0;
        for (size_t i = 0; i < LEVELS; ++i) {
            avg_price += prices[i] * quantities[i];
        }

        double total_quantity = capital_tolerance / avg_price;

        std::vector<matching::StoredOrder> orders(LEVELS);

        for (size_t i = 0; i < LEVELS; ++i) {
            auto side = (i < LEVELS / 2) ? messages::SIDE::BUY : messages::SIDE::SELL;
            auto trader = std::static_pointer_cast<manager::GenericTrader>(
                this->shared_from_this()
            );

            orders[i] = matching::StoredOrder{trader,    side,
                                              TICKER,    total_quantity * quantities[i],
                                              prices[i], current_tick};
            if (side == messages::SIDE::BUY) {
                modify_long_capital(total_quantity * quantities[i] * prices[i]);
            }
            else {
                modify_short_capital(total_quantity * quantities[i] * prices[i]);
            }
        }
        modify_open_bids(LEVELS / 2);
        modify_open_asks(LEVELS / 2);

        return orders;
    }

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
};

} // namespace bots
} // namespace nutc
