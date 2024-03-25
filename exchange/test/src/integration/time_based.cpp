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

    nutc::manager::TraderManager& users_ =
        nutc::manager::TraderManager::get_instance(); // NOLINT(*)
    nutc::engine_manager::EngineManager& engine_manager_ =
        nutc::engine_manager::EngineManager::get_instance(); // NOLINT(*)
};

TEST_F(IntegrationBasicAlgo, AlgoStartDelay)
{
    std::vector<std::string> names{"test_algos/buy_tsla_at_100"};
    if (!nutc::testing_utils::initialize_testing_clients(
            users_, names, /*has_delay=*/true
        )) {
        FAIL() << "Failed to initialize testing clients";
    }

    auto start = std::chrono::high_resolution_clock::now();

    engine_manager_.add_engine("TSLA");
    std::string user_id = users_.add_bot_trader(0);
    users_.get_trader(user_id)->modify_holdings("TSLA", 1000); // NOLINT

    rmq::RabbitMQOrderHandler::handle_incoming_market_order(
        engine_manager_,
        nutc::messages::MarketOrder{
            user_id, nutc::messages::SIDE::SELL, "TSLA", 100, 100
        }
    ); // NOLINT
    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    auto mess = rmq::RabbitMQConsumer::consume_message();

    auto end = std::chrono::high_resolution_clock::now();
    const int64_t duration_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    const double wait_time_ms = CLIENT_WAIT_SECS * 1000;

    EXPECT_GE(duration_ms, wait_time_ms);
    EXPECT_LE(duration_ms, wait_time_ms + 1000);
}
