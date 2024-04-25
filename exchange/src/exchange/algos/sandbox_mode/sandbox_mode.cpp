#include "sandbox_mode.hpp"

#include "exchange/traders/trader_types/algo_trader.hpp"
#include "shared/config/config_loader.hpp"

#include <glaze/glaze.hpp>

namespace nutc {
namespace algos {
void
SandboxAlgoInitializer::initialize_trader_container(traders::TraderContainer& traders
) const
{
    int starting_cap = config::Config::get().constants().STARTING_CAPITAL;
    // In sandbox, we aren't given their full name
    traders.add_trader<traders::LocalTrader>(
        USER_ID, "SANDBOX_USER", ALGO_ID, starting_cap
    );
}

} // namespace algos
} // namespace nutc
