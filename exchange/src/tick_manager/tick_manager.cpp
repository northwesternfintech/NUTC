#include "tick_manager.hpp"

namespace nutc {
namespace ticks {

void
TickManager::run()
{
    using namespace std::chrono;
    auto next_tick = steady_clock::now();
    while (running_) {
        next_tick += milliseconds(1000 / hz_);
        std::this_thread::sleep_until(next_tick);
        notify_tick();
    }
}

// should be inlined
void
TickManager::notify_tick()
{
    for (TickObserver* observer : observers) {
        observer->onTick();
    }
}

} // namespace ticks
} // namespace nutc
