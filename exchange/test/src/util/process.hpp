#pragma once

#include "config.h"
#include "exchange/traders/trader_container.hpp"
#include "shared/types/decimal.hpp"

namespace nutc::test {

std::vector<std::reference_wrapper<exchange::GenericTrader>> start_wrappers(
    nutc::exchange::TraderContainer& users,
    const std::vector<std::string>& algo_filenames,
    shared::decimal_price starting_capital = TEST_STARTING_CAPITAL,
    size_t start_delay = 0
);

exchange::GenericTrader& start_wrappers(
    nutc::exchange::TraderContainer& users, const std::string& filename,
    shared::decimal_price starting_capital = TEST_STARTING_CAPITAL,
    size_t start_delay = 0
);

} // namespace nutc::test
