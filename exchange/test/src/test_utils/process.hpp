#pragma once

#include "exchange/traders/trader_container.hpp"

namespace nutc {
namespace testing_utils {

void kill_all_processes(const traders::TraderContainer& users);

[[nodiscard]] bool initialize_testing_clients(
    nutc::traders::TraderContainer& users,
    const std::vector<std::string>& algo_filenames, bool has_delay = false
);

} // namespace testing_utils
} // namespace nutc
