#pragma once

#include "generic_bot.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"

namespace nutc {

namespace bots {

class RetailBot : public GenericBot {
    std::random_device rd;
    std::mt19937 gen;
    std::poisson_distribution<> poisson_dist;

public:
    RetailBot(std::string bot_id, float interest_limit) :
        GenericBot(std::move(bot_id), interest_limit)
    {}

    std::optional<messages::MarketOrder> take_action(float current, float theo);

private:
    static float calculate_order_price(
        messages::SIDE side, float current_price, float theo_price,
        float buffer_percent = 0.02f
    );
};

} // namespace bots

} // namespace nutc
