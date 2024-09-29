#include "institutional.hpp"

namespace nutc::exchange {
void
InstitutionalBot::take_action(const shared_bot_state& state)
{
    float random = dist_(gen_);
    auto quantity = get_initial_capital() / state.MIDPRICE;

    if (random < 1.0f) {
        add_market_order(common::Side::buy, quantity);
    }
    else if (random < 2.0f) {
        add_market_order(common::Side::sell, quantity);
    }
}
} // namespace nutc::exchange
