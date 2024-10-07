#include "bot_trader.hpp"

#include "common/types/decimal.hpp"

#include <cassert>

#include <random>

namespace nutc::exchange {

double
BotTrader::generate_gaussian_noise(double mean, double stddev)
{
    static std::random_device rand{};
    static std::mt19937 gen{rand()};
    static std::normal_distribution<double> distr{mean, stddev};

    return distr(gen);
}

void
BotTrader::notify_position_change(common::position order)
{
    assert(order.ticker == TICKER);

    common::decimal_price total_cap = order.price * order.quantity;
    if (order.side == common::Side::buy) {
        modify_long_capital(total_cap);
    }
    else {
        modify_short_capital(total_cap);
    }

    GenericTrader::notify_position_change(order);
}

} // namespace nutc::exchange
