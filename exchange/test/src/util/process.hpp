#pragma once

#include "config.h"
#include "exchange/traders/trader_container.hpp"

namespace nutc {
namespace test {

[[nodiscard]] std::vector<std::shared_ptr<traders::GenericTrader>> start_wrappers(
    nutc::traders::TraderContainer& users,
    const std::vector<std::string>& algo_filenames,
    double starting_capital = TEST_STARTING_CAPITAL, size_t start_delay = 0
);

[[nodiscard]] std::shared_ptr<traders::GenericTrader> start_wrappers(
    nutc::traders::TraderContainer& users, const std::string& filename,
    double starting_capital = TEST_STARTING_CAPITAL, size_t start_delay = 0
);

} // namespace test
} // namespace nutc
