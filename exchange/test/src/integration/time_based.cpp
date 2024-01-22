#include "../../../src/config.h"
#include "rabbitmq/connection_manager/RabbitMQConnectionManager.hpp"
#include "rabbitmq/consumer/RabbitMQConsumer.hpp"
#include "rabbitmq/order_handler/RabbitMQOrderHandler.hpp"
#include "test_utils/macros.hpp"
#include "test_utils/process.hpp"
#include "utils/messages.hpp"

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
    nutc::engine_manager::Manager engine_manager_; // NOLINT(*)
};

TEST_F(IntegrationBasicAlgo, AlgoStartDelay)
{
    std::vector<std::string> names{"test_algos/buy_tsla_at_100"};
    nutc::testing_utils::initialize_testing_clients(users_, names);

    auto start = std::chrono::high_resolution_clock::now();

    engine_manager_.add_engine("TSLA");
    rmq::RabbitMQOrderHandler::add_liquidity_to_ticker(
        users_, engine_manager_, "TSLA", 100, 100 // NOLINT (magic-number-*)
    );
    auto mess = rmq::RabbitMQConsumer::consume_message();

    auto end = std::chrono::high_resolution_clock::now();
    const double duration_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    const double wait_time_ms = CLIENT_WAIT_SECS * 1000;
    const double tolerance_ms = MAX_TIME_TOLERANCE_SECONDS * 1000;

    EXPECT_THAT(
        duration_ms,
        AnyOf(
            Le(tolerance_ms),
            AllOf(Ge(tolerance_ms - wait_time_ms), Le(tolerance_ms + wait_time_ms))
        )
    );
}
