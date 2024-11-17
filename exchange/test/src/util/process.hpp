#pragma once

#include "common/types/algorithm/local_algorithm.hpp"
#include "common/types/decimal.hpp"
#include "config.hpp"
#include "exchange/traders/trader_container.hpp"

namespace nutc::test {

std::vector<std::reference_wrapper<exchange::GenericTrader>> start_wrappers(
    nutc::exchange::TraderContainer& users,
    const std::vector<common::LocalAlgorithm>& algos,
    common::decimal_price starting_capital = TEST_STARTING_CAPITAL,
    size_t start_delay = 0
);

exchange::GenericTrader& start_wrappers(
    nutc::exchange::TraderContainer& users, common::AlgoLanguage language,
    const std::string& algo_name,
    common::decimal_price starting_capital = TEST_STARTING_CAPITAL,
    size_t start_delay = 0
);

} // namespace nutc::test
