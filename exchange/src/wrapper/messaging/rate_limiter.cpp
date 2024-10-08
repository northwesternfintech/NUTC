#include "rate_limiter.hpp"

namespace nutc {
namespace wrapper {
bool
RateLimiter::should_rate_limit()
{
    auto now = std::chrono::steady_clock::now();

    while (!timestamps_.empty() && (now - timestamps_.front()) > TIME_WINDOW)
        timestamps_.pop();

    if (timestamps_.size() >= MAX_CALLS)
        return true;

    timestamps_.push(now);
    return false;
}
} // namespace wrapper
} // namespace nutc
