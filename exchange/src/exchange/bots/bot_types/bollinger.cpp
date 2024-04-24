#include "bollinger.hpp"

#include "exchange/config.h"

namespace nutc {
namespace bots {

bool
BollingerBot::is_active() const
{
    return get_capital() > -get_interest_limit() * .9;
}

std::optional<messages::market_order>
BollingerBot::take_action(double current, double theo)
{
    if (!is_active()) {
        return std::nullopt;
    }

    update_statistics(current);

    // Ensure enough data to calculate Bollinger Bands.
    if (price_history.size() < sma_period) {
        return std::nullopt;
    }

    auto [lower_band, upper_band] = calculate_bollinger_bands();

    if (current < lower_band) {
        return messages::market_order{
            get_ticker(), util::Side::buy, calculate_order_size(), 0
        };
    }
    else if (current > upper_band) {
        return messages::market_order{
            get_ticker(), util::Side::sell, calculate_order_size(), 0
        };
    }

    return std::nullopt;
}

// Updates SMA and variance incrementally when a new price point is received
void
BollingerBot::update_statistics(double new_price)
{
    double old_price = 0.0;
    if (price_history.size() == sma_period) {
        old_price = price_history.front();
        price_history.pop_front();
    }

    price_history.push_back(new_price);
    current_sma += (new_price - old_price) / sma_period;

    if (old_price != 0.0) {
        current_variance += ((new_price - current_sma) * (new_price - current_sma)
                             - (old_price - current_sma) * (old_price - current_sma))
                            / sma_period;
    }
}

std::pair<double, double>
BollingerBot::calculate_bollinger_bands()
{
    double stdev = std::sqrt(current_variance);
    double lower_band = current_sma - multiplier * stdev;
    double upper_band = current_sma + multiplier * stdev;
    return {lower_band, upper_band};
}

} // namespace bots
} // namespace nutc
