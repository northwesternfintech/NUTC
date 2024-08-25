#pragma once
#include <cstdint>

#ifdef __linux__
#  include <sys/prctl.h>
#  include <sys/resource.h>

#  include <csignal>

namespace nutc::wrapper {
[[nodiscard]] inline bool
kill_on_exchange_death()
{
    return !prctl(PR_SET_PDEATHSIG, SIGKILL);
}

[[nodiscard]] inline bool
set_memory_limit(std::size_t limit_in_mb)
{
    struct rlimit limit;
    limit.rlim_cur = limit_in_mb * 1024 * 1024; // Set the soft limit.
    limit.rlim_max = limit_in_mb * 1024 * 1024; // Set the hard limit.

    if (setrlimit(RLIMIT_AS, &limit) == -1) {
        return false;
    }
    return true;
}
} // namespace nutc::wrapper
#else
namespace nutc::wrapper {
[[nodiscard]] inline bool
kill_on_exchange_death()
{
    return true;
}

[[nodiscard]] inline bool
set_memory_limit(std::size_t)
{
    return true;
}

} // namespace nutc::wrapper
#endif
