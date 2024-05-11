#include <chrono>

#include <queue>

namespace nutc {
namespace rate_limiter {
class RateLimiter {
    std::queue<std::chrono::steady_clock::time_point> timestamps_;

    // TODO(stevenewald): make configurable
    static constexpr size_t MAX_CALLS = 250;
    static constexpr std::chrono::milliseconds TIME_WINDOW = std::chrono::milliseconds(500);

public:
    bool should_rate_limit();
};
} // namespace rate_limiter
} // namespace nutc
