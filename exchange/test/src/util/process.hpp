#pragma once

#include "config.h"
#include "exchange/traders/trader_container.hpp"

namespace nutc {
namespace test {

std::vector<std::reference_wrapper<traders::GenericTrader>> start_wrappers(
    nutc::traders::TraderContainer& users,
    const std::vector<std::string>& algo_filenames,
    double starting_capital = TEST_STARTING_CAPITAL, size_t start_delay = 0
);

traders::GenericTrader& start_wrappers(
    nutc::traders::TraderContainer& users, const std::string& filename,
    double starting_capital = TEST_STARTING_CAPITAL, size_t start_delay = 0
);

} // namespace test
} // namespace nutc
