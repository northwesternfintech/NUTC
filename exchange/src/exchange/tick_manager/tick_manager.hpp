#pragma once

#include "tick_observer.hpp"

#include <list>
#include <thread>

namespace nutc {
namespace ticks {

class TickManager {
public:
    TickManager(const TickManager&) = delete;
    TickManager(TickManager&&) = delete;
    TickManager& operator=(const TickManager&) = delete;
    TickManager& operator=(TickManager&&) = delete;

    static TickManager&
    get_instance(size_t start_tick_rate)
    {
        static TickManager tm(start_tick_rate);
        return tm;
    }

    void
    attach(TickObserver* observer)
    {
        observers.push_back(observer);
    }

    void
    detach(TickObserver* observer)
    {
        observers.remove(observer);
    }

    void
    start()
    {
        running_ = true;
        tick_thread_ = std::thread(&TickManager::run, this);
    }

    void
    stop()
    {
        running_ = false;
        tick_thread_.join();
    }

private:
    size_t hz_;
    std::atomic<bool> running_;
    std::thread tick_thread_;
    std::list<TickObserver*> observers;

    TickManager(size_t start_tick_rate) : hz_(start_tick_rate) {}

    void notify_tick();
    void run();
};

} // namespace ticks
} // namespace nutc
