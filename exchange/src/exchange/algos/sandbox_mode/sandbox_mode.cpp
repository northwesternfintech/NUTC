#include "sandbox_mode.hpp"

#include "exchange/config.h"

#include <glaze/glaze.hpp>

namespace nutc {
namespace algo_mgmt {
void
SandboxAlgoManager::initialize_client_manager(manager::TraderManager& users)
{
    users.add_remote_trader(user_id_, algo_id_, STARTING_CAPITAL);
    num_clients_ = 1;
}

void
SandboxAlgoManager::initialize_files() const
{}

} // namespace algo_mgmt
} // namespace nutc
