#pragma once

#include "common/types/algorithm/algorithm.hpp"
#include "common/types/decimal.hpp"
#include "config.h"
#include "exchange/traders/trader_container.hpp"

namespace nutc::test {

std::vector<std::reference_wrapper<exchange::GenericTrader>> start_wrappers(
    nutc::exchange::TraderContainer& users,
    const std::vector<common::LocalAlgorithm>& algos,
    common::decimal_price starting_capital = TEST_STARTING_CAPITAL,
    size_t start_delay = 0
);

exchange::GenericTrader& start_wrappers(
    nutc::exchange::TraderContainer& users, const common::LocalAlgorithm& algo,
    common::decimal_price starting_capital = TEST_STARTING_CAPITAL,
    size_t start_delay = 0
);

} // namespace nutc::test
