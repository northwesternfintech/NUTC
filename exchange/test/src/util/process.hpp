#pragma once

#include "common/types/decimal.hpp"
#include "common/util.hpp"
#include "config.h"
#include "exchange/traders/trader_container.hpp"

namespace nutc::test {

std::vector<std::reference_wrapper<exchange::GenericTrader>> start_wrappers(
    nutc::exchange::TraderContainer& users,
    const std::vector<std::pair<std::string, common::AlgoType>>& algo_filenames,
    common::decimal_price starting_capital = TEST_STARTING_CAPITAL,
    size_t start_delay = 0
);

exchange::GenericTrader& start_wrappers(
    nutc::exchange::TraderContainer& users, const std::string& filename,
    common::AlgoType algo_type,
    common::decimal_price starting_capital = TEST_STARTING_CAPITAL,
    size_t start_delay = 0
);

} // namespace nutc::test
