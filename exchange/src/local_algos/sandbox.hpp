#pragma once

#include "client_manager/client_manager.hpp"

namespace nutc {
namespace sandbox {

int initialize_client_manager(manager::ClientManager& users);

void create_sandbox_algo_files();

} // namespace sandbox
} // namespace nutc
