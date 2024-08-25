#include "bot_trader.hpp"

#include "shared/types/decimal_price.hpp"

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
BotTrader::notify_position_change(shared::position order)
{
    assert(order.ticker == TICKER);

    shared::decimal_price total_cap = order.price * order.quantity;
    if (order.side == shared::Side::buy) {
        modify_long_capital(total_cap);
        modify_open_bids(order.quantity);
    }
    else {
        modify_short_capital(total_cap);
        modify_open_asks(order.quantity);
    }
}

} // namespace nutc::exchange
