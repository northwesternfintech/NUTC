#pragma once

#include "exchange/logging.hpp"
#include "tick_observer.hpp"

#include <cassert>
#include <chrono>

#include <list>
#include <thread>

namespace nutc {
namespace ticks {

enum class PRIORITY { first, second, third };

using std::chrono::milliseconds;

class TickManager {
public:
    [[nodiscard]] uint64_t
    get_current_tick() const
    {
        return current_tick_;
    }

    void
    attach(
        TickObserver* observer, PRIORITY priority, const std::string& name = "UNNAMED"
    )
    {
        log_i(
            tick_manager, "Tick engine registered observer {} with priority {}", name,
            static_cast<int>(priority)
        );
        switch (priority) {
            case PRIORITY::first:
                first_observers_.push_back(observer);
                break;
            case PRIORITY::second:
                second_observers_.push_back(observer);
                break;
            case PRIORITY::third:
                third_observers_.push_back(observer);
                break;
        }
    }

    void
    detach(TickObserver* observer, PRIORITY priority)
    {
        switch (priority) {
            case PRIORITY::first:
                first_observers_.remove(observer);
                break;
            case PRIORITY::second:
                second_observers_.remove(observer);
                break;
            case PRIORITY::third:
                third_observers_.remove(observer);
                break;
        }
    }

    struct tick_metrics_t {
        milliseconds top_1p_ms;
        milliseconds top_5p_ms;
        milliseconds top_10p_ms;
        milliseconds top_50p_ms;
        milliseconds median_tick_ms;
        milliseconds avg_tick_ms;
    };

    [[nodiscard]] tick_metrics_t get_tick_metrics() const;

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
    milliseconds delay_time_;
    std::atomic<bool> running_;
    std::thread tick_thread_;
    std::list<TickObserver*> first_observers_;
    std::list<TickObserver*> second_observers_;
    std::list<TickObserver*> third_observers_;
    static constexpr uint16_t MS_PER_SECOND = 1000;

    std::deque<milliseconds> last_1000_tick_times_;

    explicit TickManager(uint16_t start_tick_rate) :
        delay_time_(milliseconds(MS_PER_SECOND / start_tick_rate))
    {}

    auto notify_tick_();
    void run_();

public:
    TickManager(const TickManager&) = delete;
    TickManager(TickManager&&) = delete;
    TickManager& operator=(const TickManager&) = delete;
    TickManager& operator=(TickManager&&) = delete;

    static TickManager&
    get_instance(uint16_t start_tick_rate = 0)
    {
        static bool has_been_initialized = false;
        if (start_tick_rate == 0) {
            assert(has_been_initialized);
        }
        has_been_initialized = true;

        static TickManager manager(start_tick_rate);
        return manager;
    }

private:
    uint64_t current_tick_ = 0;
};
} // namespace ticks
} // namespace nutc
