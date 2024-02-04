#include "tick_manager.hpp"

namespace nutc {
namespace ticks {

void
TickManager::run_()
{
    using namespace std::chrono;
    auto next_tick = steady_clock::now();
    while (running_) {
        next_tick += milliseconds(delay_time_);
        std::this_thread::sleep_until(next_tick);
        notify_tick_();
    }
}

void
TickManager::notify_tick_()
{
    for (TickObserver* observer : observers_) {
        observer->onTick();
    }
}

} // namespace ticks
} // namespace nutc
