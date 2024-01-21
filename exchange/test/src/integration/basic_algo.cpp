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
    rmq::RabbitMQOrderHandler::add_liquidity_to_ticker(
        users_, engine_manager_, "TSLA", 100, 100 // NOLINT (magic-number-*)
    );

    auto mess = rmq::RabbitMQConsumer::consume_message();
    EXPECT_TRUE(std::holds_alternative<nutc::messages::MarketOrder>(mess));

    nutc::messages::MarketOrder actual = std::get<nutc::messages::MarketOrder>(mess);
    EXPECT_EQ_MARKET_ORDER(
        actual, "test_algos/buy_tsla_at_100", "TSLA", nutc::messages::SIDE::BUY, 100, 10
    );
}

TEST_F(IntegrationBasicAlgo, OnTradeUpdate)
{
    std::vector<std::string> names{"test_algos/buy_tsla_on_trade"};
    nutc::testing_utils::initialize_testing_clients(users_, names);

    engine_manager_.add_engine("TSLA");
    engine_manager_.add_engine("APPL");

    rmq::RabbitMQOrderHandler::add_liquidity_to_ticker(
        users_, engine_manager_, "TSLA", 100, 100 // NOLINT (magic-number-*)
    );

    // obupdate triggers one user to place a BUY order of 10 TSLA at 100
    auto mess1 = rmq::RabbitMQConsumer::consume_message();
    EXPECT_TRUE(std::holds_alternative<nutc::messages::MarketOrder>(mess1));

    nutc::messages::MarketOrder actual_mo =
        std::get<nutc::messages::MarketOrder>(mess1);
    EXPECT_EQ_MARKET_ORDER(
        actual_mo, "test_algos/buy_tsla_on_trade", "TSLA", nutc::messages::SIDE::BUY,
        102, 10
    );

    rmq::RabbitMQOrderHandler::handle_incoming_market_order(
        engine_manager_, users_, actual_mo
    );

    // on_trade_match triggers one user to place a BUY order of 1 TSLA at 100
    auto mess2 = rmq::RabbitMQConsumer::consume_message();
    EXPECT_TRUE(std::holds_alternative<nutc::messages::MarketOrder>(mess2));

    nutc::messages::MarketOrder actual2 = std::get<nutc::messages::MarketOrder>(mess2);
    EXPECT_EQ_MARKET_ORDER(
        actual2, "test_algos/buy_tsla_on_trade", "APPL", nutc::messages::SIDE::BUY, 100,
        1
    );
}

TEST_F(IntegrationBasicAlgo, OnAccountUpdate)
{
    std::vector<std::string> names{"test_algos/buy_tsla_on_account"};
    nutc::testing_utils::initialize_testing_clients(users_, names);

    engine_manager_.add_engine("TSLA");
    engine_manager_.add_engine("APPL");

    rmq::RabbitMQOrderHandler::add_liquidity_to_ticker(
        users_, engine_manager_, "TSLA", 100, 100 // NOLINT (magic-number-*)
    );

    // obupdate triggers one user to place a BUY order of 10 TSLA at 102
    auto mess1 = rmq::RabbitMQConsumer::consume_message();
    EXPECT_TRUE(std::holds_alternative<nutc::messages::MarketOrder>(mess1));

    nutc::messages::MarketOrder actual_mo =
        std::get<nutc::messages::MarketOrder>(mess1);
    EXPECT_EQ_MARKET_ORDER(
        actual_mo, "test_algos/buy_tsla_on_account", "TSLA", nutc::messages::SIDE::BUY,
        102, 10
    );

    rmq::RabbitMQOrderHandler::handle_incoming_market_order(
        engine_manager_, users_, actual_mo
    );

    // on_trade_match triggers one user to place a BUY order of 1 TSLA at 100
    auto mess2 = rmq::RabbitMQConsumer::consume_message();
    EXPECT_TRUE(std::holds_alternative<nutc::messages::MarketOrder>(mess2));

    nutc::messages::MarketOrder actual2 = std::get<nutc::messages::MarketOrder>(mess2);
    EXPECT_EQ_MARKET_ORDER(
        actual2, "test_algos/buy_tsla_on_account", "APPL", nutc::messages::SIDE::BUY,
        100, 1
    );
}
