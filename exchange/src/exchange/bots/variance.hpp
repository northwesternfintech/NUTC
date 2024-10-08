#pragma once

#include "common/types/decimal.hpp"

#include <deque>

namespace nutc::exchange {
class VarianceCalculator {
    static constexpr size_t PRICES_LOOKBACK_TICKS{1000};

    std::deque<common::decimal_price> prices;

    std::vector<double> calculate_returns() const;
    double calculate_mean(const std::vector<double>& data) const;
    double calculate_variance(const std::vector<double>& returns, double mean) const;

public:
    void record_price(common::decimal_price price);
    double calculate_volatility() const;
};

} // namespace nutc::exchange
