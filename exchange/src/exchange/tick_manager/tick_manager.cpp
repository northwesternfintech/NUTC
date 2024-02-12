#include "tick_manager.hpp"

#include "exchange/matching/manager/engine_manager.hpp"

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
    auto& engine_manager = nutc::engine_manager::EngineManager::get_instance();

    engine_manager.set_engine_state(nutc::engine_manager::EngineState::BOT);
    for (TickObserver* observer : first_observers_) {
        observer->on_tick(current_tick_);
    }

    for (TickObserver* observer : second_observers_) {
        observer->on_tick(current_tick_);
    }

    for (TickObserver* observer : third_observers_) {
        observer->on_tick(current_tick_);
    }

    for (TickObserver* observer : fourth_observers_) {
        observer->on_tick(current_tick_);
    }

    engine_manager.set_engine_state(nutc::engine_manager::EngineState::RMQ);
}

} // namespace ticks
} // namespace nutc
