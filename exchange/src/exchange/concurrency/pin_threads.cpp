#include "exchange/logging.hpp"

#ifdef __linux__
#  include <pthread.h>
#  include <sched.h>
#endif

namespace nutc {
namespace concurrency {

void
pin_to_core(size_t core_num, std::string thread_name)
{
#ifdef __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_num, &cpuset);

    pthread_t current_thread = pthread_self();
    int result = pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
    if (result != 0) {
        log_e(main, "Failed to set thread affinity");
        abort();
    }
    log_i(main, "Pinned {} thread to core {}", thread_name, core_num);
#else
    log_e(
        main, "pin_to_core not implemented for this platform. Both threads may run on "
              "the same core. This is okay for testing, but not for production."
    );
#endif
}

} // namespace concurrency
} // namespace nutc
