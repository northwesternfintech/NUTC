#include "rabbitmq/connection_manager/RabbitMQConnectionManager.hpp"
#include "rabbitmq/consumer/RabbitMQConsumer.hpp"
#include "rabbitmq/order_handler/RabbitMQOrderHandler.hpp"
#include "test_utils/macros.hpp"
#include "test_utils/process.hpp"
#include "utils/messages.hpp"

#include <gtest/gtest.h>

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

TEST_F(IntegrationBasicAlgo, InitialLiquidity)
{
    std::vector<std::string> names{"test_algos/buy_tsla_at_100"};
    nutc::testing_utils::initialize_testing_clients(users_, names);

    // want to see if it buys
    engine_manager_.add_engine("TSLA");
    rmq::RabbitMQOrderHandler::addLiquidityToTicker(
        users_, engine_manager_, "TSLA", 100, 100
    );

    auto mess = rmq::RabbitMQConsumer::consumeMessage();
    EXPECT_TRUE(std::holds_alternative<nutc::messages::MarketOrder>(mess));

    nutc::messages::MarketOrder actual = std::get<nutc::messages::MarketOrder>(mess);
    EXPECT_EQ_MARKET_ORDER(
        actual, "test_algos/buy_tsla_at_100", "TSLA", nutc::messages::SIDE::BUY, 100, 10
    );
}
