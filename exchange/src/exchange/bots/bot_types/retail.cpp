#include "retail.hpp"

#include "exchange/config/static/config.h"

namespace nutc {
namespace bots {

void
RetailBot::take_action(double midprice, double theo, double)
{
    static std::uniform_real_distribution<> dis{0.0, 1.0};

    double p_trade = (1 - get_capital_utilization());

    double noise_factor = dis(gen_);

    double signal_strength = AGGRESSIVENESS * std::fabs(theo - midprice) / midprice;

    if (noise_factor >= p_trade * signal_strength)
        return;
    if (poisson_dist_(gen_) <= 0)
        return;

    double noised_theo = theo + generate_gaussian_noise(0, .1);
    double quantity =
        (1 - get_capital_utilization()) * get_interest_limit() / noised_theo;
    quantity *= RETAIL_ORDER_SIZE;

    auto side = (midprice < noised_theo) ? util::Side::buy : util::Side::sell;

    add_market_order(side, quantity);

    return;
}

} // namespace bots
} // namespace nutc
