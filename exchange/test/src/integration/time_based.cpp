#include "exchange/rabbitmq/connection_manager/RabbitMQConnectionManager.hpp"
#include "exchange/rabbitmq/consumer/RabbitMQConsumer.hpp"
#include "exchange/rabbitmq/order_handler/RabbitMQOrderHandler.hpp"
#include "test_utils/macros.hpp"
#include "test_utils/process.hpp"

#include <gtest/gtest.h>

#include <chrono>

namespace rmq = nutc::rabbitmq;

class IntegrationBasicAlgo : public ::testing::Test {
protected:
    void
    SetUp() override
    {
        auto& rmq_conn = rmq::RabbitMQConnectionManager::get_instance();

        if (!rmq_conn.connected_to_rabbitmq()) {
            FAIL() << "Failed to connect to rabbitmq";
        }
    }

    void
    TearDown() override
    {
        nutc::testing_utils::kill_all_processes(users_);
        rmq::RabbitMQConnectionManager::reset_instance();
    }

    nutc::manager::ClientManager users_;           // NOLINT(*)
    nutc::engine_manager::EngineManager& engine_manager_ = nutc::engine_manager::EngineManager::get_instance(); // NOLINT(*)
};

TEST_F(IntegrationBasicAlgo, AlgoStartDelay)
{
    std::vector<std::string> names{"test_algos/buy_tsla_at_100"};
    nutc::testing_utils::initialize_testing_clients(
        users_, names, nutc::client::SpawnMode::NORMAL
    );

    auto start = std::chrono::high_resolution_clock::now();

    engine_manager_.add_engine("TSLA");
    rmq::RabbitMQOrderHandler::add_liquidity_to_ticker(
        users_, engine_manager_, "TSLA", 100, 100 // NOLINT (magic-number-*)
    );
    auto mess = rmq::RabbitMQConsumer::consume_message();

    auto end = std::chrono::high_resolution_clock::now();
    const int64_t duration_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    const double wait_time_ms = CLIENT_WAIT_SECS * 1000;

    EXPECT_GE(duration_ms, wait_time_ms);
    EXPECT_LE(duration_ms, wait_time_ms + 1000);
}
