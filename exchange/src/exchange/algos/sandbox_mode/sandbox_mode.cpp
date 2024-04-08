#include "sandbox_mode.hpp"

#include "exchange/traders/trader_types/remote_trader.hpp"
#include "shared/config/config_loader.hpp"

#include <glaze/glaze.hpp>

namespace nutc {
namespace algo_mgmt {
void
SandboxAlgoManager::initialize_client_manager(manager::TraderManager& users)
{
    int starting_cap = config::Config::get_instance().constants().STARTING_CAPITAL;
    // In sandbox, we aren't given their full name
    users.add_trader<manager::RemoteTrader>(
        user_id_, "UNKNOWN", algo_id_, starting_cap
    );
}

void
SandboxAlgoManager::initialize_files() const
{}

} // namespace algo_mgmt
} // namespace nutc
