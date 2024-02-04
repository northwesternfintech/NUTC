#include "exchange/tick_manager/tick_manager.hpp"

#include "exchange/tick_manager/tick_observer.hpp"

#include <gtest/gtest.h>

// NOLINTBEGIN

class UnitTickManagerTest : public ::testing::Test {
    using TickManager = nutc::ticks::TickManager;

protected:
    static constexpr uint16_t START_TICK_RATE = 100;

    void
    SetUp() override
    {}

    TickManager& manager_ = TickManager::get_instance(START_TICK_RATE);
};

TEST_F(UnitTickManagerTest, SingletonInstance)
{
    manager_.start();
    auto& instance1 = nutc::ticks::TickManager::get_instance(60);
    auto& instance2 = nutc::ticks::TickManager::get_instance(30);
    ASSERT_EQ(std::addressof(instance1), std::addressof(instance2));
    manager_.stop();
}

class TestObserver : public nutc::ticks::TickObserver {
public:
    void
    on_tick() override
    {
        tick_count_++;
    }

    uint32_t tick_count_ = 0;

    uint32_t
    get_tick_count()
    {
        return tick_count_;
    }
};

TEST_F(UnitTickManagerTest, AttachDetachObserver)
{
    TestObserver observer;
    manager_.attach(&observer);
    manager_.start();
    // wait for 100 ms, should be around 10 ticks
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    manager_.stop();
    manager_.detach(&observer);
    ASSERT_GE(observer.get_tick_count(), 8);
    ASSERT_LE(observer.get_tick_count(), 12);
}

// NOLINTEND
