#pragma once

#ifdef __linux__
#  include <sys/resource.h>

namespace nutc {
namespace limits {
bool
set_memory_limit(size_t limit_in_mb)
{
    struct rlimit limit;
    limit.rlim_cur = limit_in_mb * 1024 * 1024; // Set the soft limit.
    limit.rlim_max = limit_in_mb * 1024 * 1024; // Set the hard limit.

    if (setrlimit(RLIMIT_AS, &limit) == -1) {
        return false;
    }
    return true;
}
} // namespace limits
} // namespace nutc
#else
namespace nutc {
namespace limits {
bool
set_memory_limit(size_t)
{
    return false;
}

} // namespace limits
} // namespace nutc
#endif
