#include "sandbox_mode.hpp"

#include "exchange/config.h"
#include "exchange/traders/trader_types/remote_trader.hpp"

#include <glaze/glaze.hpp>

namespace nutc {
namespace algo_mgmt {
void
SandboxAlgoManager::initialize_client_manager(manager::TraderManager& users)
{
    // In sandbox, we aren't given their full name
    users.add_trader<manager::RemoteTrader>(
        user_id_, "UNKNOWN", algo_id_, STARTING_CAPITAL
    );
    num_clients_ = 1;
}

void
SandboxAlgoManager::initialize_files() const
{}

} // namespace algo_mgmt
} // namespace nutc
