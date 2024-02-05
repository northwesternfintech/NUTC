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
        current_tick_++;
        notify_tick_();
    }
}

void
TickManager::notify_tick_()
{
    for (TickObserver* observer : first_observers_) {
        observer->on_tick(current_tick_);
    }

    for (TickObserver* observer : second_observers_) {
        observer->on_tick(current_tick_);
    }
}

} // namespace ticks
} // namespace nutc
