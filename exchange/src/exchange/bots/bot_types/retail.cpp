#include "retail.hpp"

#include "exchange/bots/shared_bot_state.hpp"
#include "exchange/config/static/config.hpp"

namespace nutc {
namespace bots {

void
RetailBot::take_action(const shared_bot_state& state)
{
    static std::uniform_real_distribution<> dis{0.0, 1.0};

    double p_trade = (1 - get_capital_utilization());

    double noise_factor = dis(gen_);

    double signal_strength =
        AGGRESSIVENESS * std::fabs(state.THEO - state.MIDPRICE) / state.MIDPRICE;

    if (noise_factor >= p_trade * signal_strength)
        return;
    if (poisson_dist_(gen_) <= 0)
        return;

    double noised_theo = state.THEO + generate_gaussian_noise(0, .1);
    double quantity =
        (1 - get_capital_utilization()) * get_interest_limit() / noised_theo;
    quantity *= RETAIL_ORDER_SIZE;

    auto side = (state.MIDPRICE < noised_theo) ? util::Side::buy : util::Side::sell;

    add_market_order(side, quantity);

    return;
}

} // namespace bots
} // namespace nutc
