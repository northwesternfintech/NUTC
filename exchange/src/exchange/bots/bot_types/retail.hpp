#pragma once

#include "exchange/traders/trader_types/bot_trader.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

namespace nutc {

namespace bots {

class RetailBot : public BotTrader {
    std::random_device rd{};
    std::mt19937 gen{};
    std::poisson_distribution<> poisson_dist{};

public:
    RetailBot(std::string ticker, double interest_limit) :
        BotTrader(std::move(ticker), interest_limit),
        AGGRESSIVENESS(std::normal_distribution<>{50, 2000}(gen))
    {}

    RetailBot(RetailBot&& other) noexcept :
        BotTrader(std::move(other)), AGGRESSIVENESS(other.AGGRESSIVENESS)
    {}

    bool constexpr can_leverage() const override {return true;}

    [[nodiscard]] bool is_active() const override;

    std::optional<messages::MarketOrder> take_action(double current, double theo);

private:
    static double calculate_order_price(
        messages::SIDE side, double current_price, double theo_price,
        double buffer_percent = 0.02
    );

    const double AGGRESSIVENESS;
};

} // namespace bots

} // namespace nutc
