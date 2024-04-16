#include "sandbox_mode.hpp"

#include "exchange/traders/trader_types/remote_trader.hpp"
#include "shared/config/config_loader.hpp"

#include <glaze/glaze.hpp>

namespace nutc {
namespace algos {
void
SandboxAlgoInitializer::initialize_trader_container(manager::TraderManager& traders
) const
{
    int starting_cap = config::Config::get_instance().constants().STARTING_CAPITAL;
    // In sandbox, we aren't given their full name
    traders.add_trader<manager::RemoteTrader>(
        user_id_, "UNKNOWN", algo_id_, starting_cap
    );
}

} // namespace algos
} // namespace nutc
