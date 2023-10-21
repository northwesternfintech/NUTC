#include <chrono>

#include <iostream>
#include <queue>
#include <stdexcept>

namespace nutc {
namespace rate_limiter {
class RateLimiter {
private:
    std::queue<std::chrono::steady_clock::time_point> timestamps;
    const size_t MAX_CALLS = 30;
    const std::chrono::seconds TIME_WINDOW = std::chrono::seconds(60);

public:
    bool should_rate_limit();
};
} // namespace rate_limiter
} // namespace nutc
