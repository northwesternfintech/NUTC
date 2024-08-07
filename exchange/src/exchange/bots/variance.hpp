#pragma once

#include "shared/types/decimal_price.hpp"

#include <deque>

namespace nutc {
namespace bots {
class VarianceCalculator {
    static constexpr size_t PRICES_LOOKBACK_TICKS{1000};

    std::deque<util::decimal_price> prices;

    std::vector<double> calculate_returns() const;
    double calculate_mean(const std::vector<double>& data) const;
    double calculate_variance(const std::vector<double>& returns, double mean) const;

public:
    void record_price(util::decimal_price price);
    double calculate_volatility() const;
};

} // namespace bots
} // namespace nutc
