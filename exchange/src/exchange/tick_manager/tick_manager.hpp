#pragma once

#include "tick_observer.hpp"

#include <chrono>

#include <list>
#include <thread>

namespace nutc {
namespace ticks {

class TickManager {
public:
    void
    attach(TickObserver* observer)
    {
        observers_.push_back(observer);
    }

    void
    detach(TickObserver* observer)
    {
        observers_.remove(observer);
    }

    void
    start()
    {
        running_ = true;
        tick_thread_ = std::thread(&TickManager::run_, this);
    }

    void
    stop()
    {
        running_ = false;
        tick_thread_.join();
    }

private:
    std::chrono::milliseconds delay_time_;
    std::atomic<bool> running_;
    std::thread tick_thread_;
    std::list<TickObserver*> observers_;
    static constexpr uint16_t MS_PER_SECOND = 1000;

    explicit TickManager(uint16_t start_tick_rate) :
        delay_time_(std::chrono::milliseconds(MS_PER_SECOND / start_tick_rate))
    {}

    void notify_tick_();
    void run_();

public:
    TickManager(const TickManager&) = delete;
    TickManager(TickManager&&) = delete;
    TickManager& operator=(const TickManager&) = delete;
    TickManager& operator=(TickManager&&) = delete;

    static TickManager&
    get_instance(uint16_t start_tick_rate)
    {
        static TickManager manager(start_tick_rate);
        return manager;
    }
};

} // namespace ticks
} // namespace nutc
