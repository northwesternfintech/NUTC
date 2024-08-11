#pragma once
#include <cstdint>

#ifdef __linux__
#  include <sys/resource.h>

namespace nutc::limits {
inline bool
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
} // namespace nutc::limits
#else
namespace nutc::limits {
bool
set_memory_limit(std::size_t)
{
    return false;
}

} // namespace nutc::limits
#endif
