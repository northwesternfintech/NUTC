#pragma once

#include "generic_bot.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

namespace nutc {

namespace bots {

class RetailBot : public GenericBot {
    std::random_device rd{};
    std::mt19937 gen{};
    std::poisson_distribution<> poisson_dist{};

public:
    RetailBot(std::string bot_id, double interest_limit) :
        GenericBot(std::move(bot_id), interest_limit),
        AGGRESSIVENESS(std::normal_distribution<>{50, 2000}(gen))
    {}

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
