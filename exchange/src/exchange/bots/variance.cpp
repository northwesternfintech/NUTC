#include "variance.hpp"

namespace nutc::exchange {

void
VarianceCalculator::record_price(common::decimal_price price)
{
    if (price == common::decimal_price{0.0})
        return;

    prices.push_back(price);

    if (prices.size() > PRICES_LOOKBACK_TICKS)
        prices.pop_front();
}

std::vector<double>
VarianceCalculator::calculate_returns() const
{
    std::vector<double> returns;
    returns.reserve(prices.size() - 1);

    for (auto price = prices.begin() + 1; price != prices.end(); price++) {
        double tickReturn =
            (double{*price} - double{*(price - 1)}) / double{*(price - 1)};
        returns.push_back(tickReturn);
    }
    return returns;
}

double
VarianceCalculator::calculate_mean(const std::vector<double>& data) const
{
    double sum{0};
    for (double d : data) {
        sum += d;
    }
    return sum / static_cast<double>(data.size());
}

double
VarianceCalculator::calculate_variance(const std::vector<double>& returns, double mean)
    const
{
    double variance = 0.0;
    for (double r : returns) {
        variance += (r - mean) * (r - mean);
    }
    return variance / static_cast<double>(returns.size() - 1);
}

double
VarianceCalculator::calculate_volatility() const
{
    if (prices.size() < 10)
        return 0;

    std::vector<double> returns = calculate_returns();
    double mean = calculate_mean(returns);
    double variance = calculate_variance(returns, mean);
    return std::sqrt(variance);
}
} // namespace nutc::exchange
