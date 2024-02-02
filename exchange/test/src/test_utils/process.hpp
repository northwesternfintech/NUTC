#pragma once

#include "exchange/client_manager/client_manager.hpp"
#include "exchange/process_spawning/spawning.hpp"

namespace nutc {
namespace testing_utils {

void kill_all_processes(const manager::ClientManager& users);

void initialize_testing_clients(
    nutc::manager::ClientManager& users, const std::vector<std::string>& algo_filenames,
    client::SpawnMode mode = client::SpawnMode::TESTING
);

} // namespace testing_utils
} // namespace nutc
