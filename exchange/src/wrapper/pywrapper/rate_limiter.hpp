#include <chrono>

#include <queue>

namespace nutc {
namespace rate_limiter {
class RateLimiter {
    std::queue<std::chrono::steady_clock::time_point> timestamps_;

    // TODO(stevenewald): make configurable
    static constexpr size_t MAX_CALLS = 10;
    static constexpr std::chrono::seconds TIME_WINDOW = std::chrono::seconds(1);

public:
    bool should_rate_limit();
};
} // namespace rate_limiter
} // namespace nutc
