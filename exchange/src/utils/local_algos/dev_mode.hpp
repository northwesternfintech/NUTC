#pragma once

#include "client_manager/client_manager.hpp"

namespace nutc {
namespace dev_mode {
void create_mt_algo_files(int num_users) noexcept;

void initialize_client_manager(manager::ClientManager& users, int num_users);

} // namespace dev_mode
} // namespace nutc
