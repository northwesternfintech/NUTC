#include "retail.hpp"

#include "common/types/decimal.hpp"
#include "exchange/bots/shared_bot_state.hpp"
#include "exchange/config/static/config.hpp"

namespace nutc::exchange {

void
RetailBot::take_action(const shared_bot_state& state)
{
    static std::uniform_real_distribution<> dis{0.0, 1};

    auto p_trade = common::decimal_price{1.0} - get_capital_utilization();

    double noise_factor = dis(gen_);

    common::decimal_price signal_strength =
        state.MIDPRICE == 0.0
            ? 0.0
            : AGGRESSIVENESS * state.THEO.difference(state.MIDPRICE) / state.MIDPRICE;

    if (noise_factor >= p_trade * signal_strength)
        return;

    common::decimal_price noised_theo = state.THEO + generate_gaussian_noise(0, .1);
    common::decimal_quantity quantity{
        (common::decimal_price{1.0} - get_capital_utilization()) * get_interest_limit()
        / noised_theo
    };
    quantity *= RETAIL_ORDER_SIZE;

    auto side = (state.MIDPRICE < noised_theo) ? common::Side::buy : common::Side::sell;

    add_market_order(side, quantity);
}

} // namespace nutc::exchange
