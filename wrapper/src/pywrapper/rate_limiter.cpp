#include "rate_limiter.hpp"

namespace nutc {
namespace rate_limiter {
bool
RateLimiter::should_rate_limit()
{
    auto now = std::chrono::steady_clock::now();

    while (!timestamps.empty() && (now - timestamps.front()) > TIME_WINDOW) {
        timestamps.pop();
    }

    if (timestamps.size() >= MAX_CALLS) {
        return true;
    }

    timestamps.push(now);
    return false;
}
} // namespace rate_limiter
} // namespace nutc
