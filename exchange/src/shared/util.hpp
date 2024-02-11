#pragma once

#include <cmath>

namespace nutc {
namespace messages {
enum class SIDE { BUY, SELL };
}

namespace util {
constexpr bool
is_close_to_zero(double value, double epsilon = 1e-6f)
{
    return std::fabs(value) < epsilon;
}

} // namespace util
} // namespace nutc
