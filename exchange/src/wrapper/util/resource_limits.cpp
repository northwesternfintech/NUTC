#include "resource_limits.hpp"

#include <sys/prctl.h>
#include <sys/resource.h>

#include <csignal>

#include <stdexcept>

namespace nutc::wrapper {
void
kill_on_exchange_death()
{
    if (prctl(PR_SET_PDEATHSIG, SIGKILL) != 0) {
        throw std::runtime_error("Failed to set kill state on parent process death");
    }
}

void
set_memory_limit(std::size_t limit_in_mb)
{
    static constexpr std::size_t BYTES_PER_MB = 1024L * 1024L;
    std::size_t limit_in_bytes = limit_in_mb * BYTES_PER_MB;

    struct rlimit limit {
        limit_in_bytes, limit_in_bytes
    };

    if (setrlimit(RLIMIT_AS, &limit) != 0) {
        throw std::runtime_error("Failed to set process memory limit");
    }
}
} // namespace nutc::wrapper
