#include "bot_trader.hpp"

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

} // namespace nutc::exchange
