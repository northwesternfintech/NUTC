#include "pin_threads.hpp"
#include "exchange/logging.hpp"

#ifdef __linux__
#  include <pthread.h>
#  include <sched.h>
#endif

namespace nutc {
namespace concurrency {

// TODO: name thread
#ifdef __linux__
void
pin_to_core(size_t core_num, const std::string& thread_name)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);

    auto num_cores = static_cast<size_t>(sysconf(_SC_NPROCESSORS_ONLN));
    size_t target_core = core_num < num_cores ? core_num : num_cores - 1;

    CPU_SET(target_core, &cpuset);

    pthread_t current_thread = pthread_self();
    int result = pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
    if (result != 0) {
        log_e(main, "Failed to set thread affinity");
        abort();
    }
    log_i(main, "Pinned {} thread to core {}", thread_name, core_num);
}
#else
void
pin_to_core(size_t, const std::string&)
{
    log_e(
        main, "pin_to_core not implemented for this platform. Both threads may run on "
              "the same core. This is okay for testing, but not for production."
    );
}
#endif

void
pin_to_core(size_t core_num)
{
    pin_to_core(core_num, "");
}

} // namespace concurrency
} // namespace nutc
