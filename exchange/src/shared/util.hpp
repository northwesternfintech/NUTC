#pragma once

#include <cmath>

#include <string>

namespace nutc {
namespace util {
enum class Side { buy, sell };
enum class Mode { sandbox, dev, normal, bots_only };

struct algorithm {
    const std::string UID;
    const std::string ALGO_ID;
};

constexpr bool
is_close_to_zero(double value, double epsilon = 1e-6)
{
    return std::fabs(value) < epsilon;
}

} // namespace util
} // namespace nutc
