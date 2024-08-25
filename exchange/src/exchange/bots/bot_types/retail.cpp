#include "retail.hpp"

#include "exchange/bots/shared_bot_state.hpp"
#include "exchange/config/static/config.hpp"
#include "shared/types/decimal_price.hpp"

namespace nutc::exchange {

void
RetailBot::take_action(const shared_bot_state& state)
{
    static std::uniform_real_distribution<> dis{0.0, 1.0};

    shared::decimal_price p_trade =
        (shared::decimal_price{1.0} - get_capital_utilization());

    double noise_factor = dis(gen_);

    shared::decimal_price signal_strength =
        AGGRESSIVENESS * state.THEO.difference(state.MIDPRICE) / state.MIDPRICE;

    if (noise_factor >= p_trade * signal_strength)
        return;
    if (poisson_dist_(gen_) <= 0)
        return;

    shared::decimal_price noised_theo = state.THEO + generate_gaussian_noise(0, .1);
    double quantity{
        (shared::decimal_price{1.0} - get_capital_utilization()) * get_interest_limit()
        / noised_theo
    };
    quantity *= RETAIL_ORDER_SIZE;

    auto side = (state.MIDPRICE < noised_theo) ? shared::Side::buy : shared::Side::sell;

    add_market_order(side, quantity);

    return;
}

} // namespace nutc::exchange
