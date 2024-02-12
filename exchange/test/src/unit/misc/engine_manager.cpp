#include "exchange/matching/manager/engine_manager.hpp"

#include <gtest/gtest.h>

// NOLINTBEGIN

class UnitEngineManagerTest : public ::testing::Test {
public:
    using EngineManager = nutc::engine_manager::EngineManager;
    using EngineState = nutc::engine_manager::EngineState;

protected:
    void
    SetUp() override
    {}

    EngineManager & manager_ = EngineManager::get_instance();
};

TEST_F(UnitEngineManagerTest, SynchronizeRMQAndBot)
{
std::atomic<bool> testingThreadDone{false};

std::thread testingThread([&]() {
    manager_.set_engine_state(EngineState::RMQ);
    manager_.get_engine("TSLA", EngineState::BOT);
    testingThreadDone = true;
});

std::this_thread::sleep_for(std::chrono::milliseconds(200));
ASSERT_EQ(testingThreadDone.load(), false);
manager_.set_engine_state(EngineState::BOT);
std::this_thread::sleep_for(std::chrono::milliseconds(200));
ASSERT_EQ(testingThreadDone.load(), true);
testingThread.detach();
}

// NOLINTEND
