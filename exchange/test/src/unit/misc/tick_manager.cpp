#include "exchange/tick_manager/tick_manager.hpp"

#include "exchange/tick_manager/tick_observer.hpp"

#include <gtest/gtest.h>

// NOLINTBEGIN

class UnitTickManagerTest : public ::testing::Test {
    using TickManager = nutc::ticks::TickManager;

protected:
    using PRIORITY = nutc::ticks::PRIORITY;
    static constexpr uint16_t START_TICK_RATE = 100;

    void
    SetUp() override
    {}

    TickManager& manager_ = TickManager::get_instance();
};

class TestObserver : public nutc::ticks::TickObserver {
public:
    void
    on_tick(uint64_t tick) override
    {
        tick_count_++;
        current_tick_ = tick;
    }

    uint32_t tick_count_ = 0;
    uint64_t current_tick_ = 0;

    uint32_t
    get_tick_count()
    {
        return tick_count_;
    }

    uint64_t
    get_current_tick()
    {
        return current_tick_;
    }
};

TEST_F(UnitTickManagerTest, AttachDetachObserver)
{
    TestObserver observer;
    manager_.attach(&observer, PRIORITY::first);
    manager_.start(START_TICK_RATE);
    // wait for 100 ms, should be around 10 ticks
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    manager_.stop();
    manager_.detach(&observer, PRIORITY::first);
    ASSERT_GE(observer.get_tick_count(), 8);
    ASSERT_LE(observer.get_tick_count(), 12);
    ASSERT_GE(observer.get_current_tick(), 8);
    ASSERT_LE(observer.get_current_tick(), 12);
}

// NOLINTEND
