#pragma once

#include "exchange/traders/trader_manager.hpp"

namespace nutc {
namespace testing_utils {

void kill_all_processes(const manager::ClientManager& users);

[[nodiscard]] bool initialize_testing_clients(
    nutc::manager::ClientManager& users, const std::vector<std::string>& algo_filenames,
    bool has_delay = false
);

} // namespace testing_utils
} // namespace nutc
