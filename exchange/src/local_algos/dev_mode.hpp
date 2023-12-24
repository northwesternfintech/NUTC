#pragma once

#include "client_manager/client_manager.hpp"

#include <vector>

namespace nutc {
namespace dev_mode {
[[nodiscard]] bool create_mt_algo_files(int num_users);

void initialize_client_manager(manager::ClientManager& users, int num_users);

void init_client_manager_from_filenames(
    manager::ClientManager& users, std::vector<std::string>& names
);

} // namespace dev_mode
} // namespace nutc
