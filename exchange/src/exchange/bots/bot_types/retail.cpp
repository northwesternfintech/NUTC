#include "retail.hpp"

#include "exchange/config.h"

namespace nutc {
namespace bots {

bool
RetailBot::is_active() const
{
    return get_capital()
           > -get_interest_limit() * .9; // drop out if they lose 90% of their money
}

void
RetailBot::take_action(double midprice, double theo)
{
    if (!is_active())
        return;

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
        add_order(util::Side::buy, quantity, price);
    }
    else if (midprice > noised_theo) {
        double price = noised_theo - RETAIL_ORDER_OFFSET;
        assert(price > 0);
        double quantity =
            (1 - get_capital_utilization()) * get_interest_limit() / price;
        quantity *= RETAIL_ORDER_SIZE;
        add_order(util::Side::sell, quantity, price);
    }
}

} // namespace bots
} // namespace nutc
