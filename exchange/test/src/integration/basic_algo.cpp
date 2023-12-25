#include "local_algos/dev_mode.hpp"
#include "process_spawning/spawning.hpp"
#include "rabbitmq/rabbitmq.hpp"
#include "test_utils/macros.hpp"
#include "test_utils/process.hpp"
#include "utils/messages.hpp"

#include <gtest/gtest.h>

namespace rmq = nutc::rabbitmq;

class IntegrationBasicAlgo : public ::testing::Test {
protected:
    // TODO: teardown of rmq connection if necessary
    void
    SetUp() override
    {
        auto& rmq_conn = rmq::RabbitMQConnectionManager::getInstance();

        if (!rmq_conn.connectedToRMQ()) {
            log_e(rabbitmq, "Failed to initialize connection");
            exit(1);
        }
    }

    void
    TearDown() override
    {
        nutc::testing_utils::kill_all_processes(users);
        rmq::RabbitMQConnectionManager::resetInstance();
    }

    nutc::manager::ClientManager users;
    nutc::engine_manager::Manager engine_manager;
};

TEST_F(IntegrationBasicAlgo, InitialLiquidity)
{
    std::vector<std::string> names{"test_algos/buy_tsla_at_100"};
    nutc::testing_utils::initialize_testing_clients(users, names);

    // want to see if it buys
    engine_manager.add_engine("TSLA");
    rmq::RabbitMQOrderHandler::addLiquidityToTicker(
        users, engine_manager, "TSLA", 100, 100
    );

    auto mess = rmq::RabbitMQConsumer::consumeMessage();
    EXPECT_TRUE(std::holds_alternative<nutc::messages::MarketOrder>(mess));

    nutc::messages::MarketOrder actual = std::get<nutc::messages::MarketOrder>(mess);
    EXPECT_EQ_MARKET_ORDER(
        actual, "test_algos/buy_tsla_at_100", "TSLA", nutc::messages::SIDE::BUY, 100, 10
    );
}
