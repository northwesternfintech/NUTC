#include "rate_limiter.hpp"

namespace nutc {
namespace rate_limiter {
bool
RateLimiter::ensureRate()
{
    auto now = std::chrono::steady_clock::now();

    while (!timestamps.empty() && (now - timestamps.front()) > TIME_WINDOW) {
        timestamps.pop();
    }

    if (timestamps.size() >= MAX_CALLS) {
        return false;
    }

    timestamps.push(now);
    return true;
}
} // namespace rate_limiter
} // namespace nutc
