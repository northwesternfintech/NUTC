#pragma once

#include "client_manager/client_manager.hpp"

namespace nutc {
namespace testing_utils {

void kill_all_processes(const manager::ClientManager& users);

void initialize_testing_clients(nutc::manager::ClientManager& users, const std::vector<std::string>& algo_filenames);

}
} // namespace nutc
