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

bool set_cpu_affinity(int cpu) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu, &mask);

    if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
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
