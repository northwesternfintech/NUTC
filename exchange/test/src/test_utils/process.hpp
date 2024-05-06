#pragma once

#include "exchange/traders/trader_container.hpp"

namespace nutc {
namespace test_utils {

[[nodiscard]] std::vector<std::shared_ptr<traders::GenericTrader>>
initialize_testing_clients(
    nutc::traders::TraderContainer& users,
    const std::vector<std::string>& algo_filenames, size_t start_delay
);

[[nodiscard]] std::vector<std::shared_ptr<traders::GenericTrader>>
initialize_testing_clients(
    nutc::traders::TraderContainer& users,
    const std::vector<std::string>& algo_filenames
);

} // namespace test_utils
} // namespace nutc
