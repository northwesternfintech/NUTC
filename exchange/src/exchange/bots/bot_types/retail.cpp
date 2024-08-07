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

    if (midprice < noised_theo) {
        double price = noised_theo + RETAIL_ORDER_OFFSET;
        assert(price > 0);
        double quantity =
            (1 - get_capital_utilization()) * get_interest_limit() / price;
        quantity *= RETAIL_ORDER_SIZE;
        add_order(util::Side::buy, price, quantity, true);
    }
    else if (midprice > noised_theo) {
        double price = noised_theo - RETAIL_ORDER_OFFSET;
        assert(price > 0);
        double quantity =
            (1 - get_capital_utilization()) * get_interest_limit() / price;
        quantity *= RETAIL_ORDER_SIZE;
        add_order(util::Side::sell, price, quantity, true);
    }
    return;
}

} // namespace bots
} // namespace nutc
