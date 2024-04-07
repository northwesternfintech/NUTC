#pragma once

#include <cmath>

#include <string>

namespace nutc {
namespace messages {
enum class SIDE { BUY, SELL };
}

namespace util {
enum class Mode { SANDBOX, DEV, NORMAL, BOTS_ONLY };

struct algorithm {
    std::string uid;
    std::string algo_id;
};

constexpr bool
is_close_to_zero(double value, double epsilon = 1e-6f)
{
    return std::fabs(value) < epsilon;
}

} // namespace util
} // namespace nutc
