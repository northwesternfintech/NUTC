#include "exchange/rabbitmq/connection_manager/RabbitMQConnectionManager.hpp"
#include "exchange/rabbitmq/consumer/RabbitMQConsumer.hpp"
#include "exchange/rabbitmq/order_handler/RabbitMQOrderHandler.hpp"
#include "exchange/tickers/manager/ticker_manager.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"
#include "test_utils/macros.hpp"
#include "test_utils/process.hpp"

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
        users_.reset();
    }

    TraderManager& users_ = nutc::manager::TraderManager::get_instance(); // NOLINT(*)
    nutc::engine_manager::EngineManager& engine_manager_ =
        nutc::engine_manager::EngineManager::get_instance(); // NOLINT(*)
};

TEST_F(IntegrationBasicAlgo, InitialLiquidity)
{
    std::vector<std::string> names{"test_algos/buy_tsla_at_100"};
    if (!nutc::testing_utils::initialize_testing_clients(users_, names)) {
        FAIL() << "Failed to initialize testing clients";
    }

    // want to see if it buys
    engine_manager_.add_engine("TSLA");

    std::string user_id = users_.add_bot_trader(0);
    users_.get_trader(user_id)->modify_holdings("TSLA", 1000); // NOLINT

    rmq::RabbitMQOrderHandler::handle_incoming_market_order(
        engine_manager_,
        nutc::messages::MarketOrder{
            user_id, nutc::messages::SIDE::SELL, "TSLA", 100, 100
        }
    );

    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    auto mess = rmq::RabbitMQConsumer::consume_message();
    ASSERT_TRUE(std::holds_alternative<nutc::messages::MarketOrder>(mess));

    nutc::messages::MarketOrder actual = std::get<nutc::messages::MarketOrder>(mess);
    ASSERT_EQ_MARKET_ORDER(
        actual, "test_algos/buy_tsla_at_100", "TSLA", nutc::messages::SIDE::BUY, 100, 10
    );
}

TEST_F(IntegrationBasicAlgo, OnTradeUpdate)
{
    std::vector<std::string> names{"test_algos/buy_tsla_on_trade"};
    if (!nutc::testing_utils::initialize_testing_clients(users_, names)) {
        FAIL() << "Failed to initialize testing clients";
    }

    engine_manager_.add_engine("TSLA");
    engine_manager_.add_engine("APPL");

    std::string user_id = users_.add_bot_trader(0);
    users_.get_trader(user_id)->modify_holdings("TSLA", 1000); // NOLINT

    rmq::RabbitMQOrderHandler::handle_incoming_market_order(
        engine_manager_,
        nutc::messages::MarketOrder{
            user_id, nutc::messages::SIDE::SELL, "TSLA", 100, 100
        }
    ); // NOLINT
    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    // obupdate triggers one user to place a BUY order of 10 TSLA at 100
    auto mess1 = rmq::RabbitMQConsumer::consume_message();
    EXPECT_TRUE(std::holds_alternative<nutc::messages::MarketOrder>(mess1));

    nutc::messages::MarketOrder actual_mo =
        std::get<nutc::messages::MarketOrder>(mess1);
    ASSERT_EQ_MARKET_ORDER(
        actual_mo, "test_algos/buy_tsla_on_trade", "TSLA", nutc::messages::SIDE::BUY,
        102, 10
    );

    rmq::RabbitMQOrderHandler::handle_incoming_market_order(
        engine_manager_, std::move(actual_mo)
    );
    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    // on_trade_match triggers one user to place a BUY order of 1 TSLA at 100
    auto mess2 = rmq::RabbitMQConsumer::consume_message();
    EXPECT_TRUE(std::holds_alternative<nutc::messages::MarketOrder>(mess2));

    nutc::messages::MarketOrder actual2 = std::get<nutc::messages::MarketOrder>(mess2);
    ASSERT_EQ_MARKET_ORDER(
        actual2, "test_algos/buy_tsla_on_trade", "APPL", nutc::messages::SIDE::BUY, 100,
        1
    );
}

TEST_F(IntegrationBasicAlgo, OnAccountUpdate)
{
    std::vector<std::string> names{"test_algos/buy_tsla_on_account"};
    if (!nutc::testing_utils::initialize_testing_clients(users_, names)) {
        FAIL() << "Failed to initialize testing clients";
    }

    engine_manager_.add_engine("TSLA");
    engine_manager_.add_engine("APPL");

    std::string user_id = users_.add_bot_trader(0);
    users_.get_trader(user_id)->modify_holdings("TSLA", 1000); // NOLINT

    rmq::RabbitMQOrderHandler::handle_incoming_market_order(
        engine_manager_,
        nutc::messages::MarketOrder{
            user_id, nutc::messages::SIDE::SELL, "TSLA", 100, 100
        }
    ); // NOLINT
    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    // obupdate triggers one user to place a BUY order of 10 TSLA at 102
    auto mess1 = rmq::RabbitMQConsumer::consume_message();
    EXPECT_TRUE(std::holds_alternative<nutc::messages::MarketOrder>(mess1));

    nutc::messages::MarketOrder actual_mo =
        std::get<nutc::messages::MarketOrder>(mess1);
    ASSERT_EQ_MARKET_ORDER(
        actual_mo, "test_algos/buy_tsla_on_account", "TSLA", nutc::messages::SIDE::BUY,
        102, 10
    );

    rmq::RabbitMQOrderHandler::handle_incoming_market_order(
        engine_manager_, std::move(actual_mo)
    );
    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    // on_trade_match triggers one user to place a BUY order of 1 TSLA at 100
    auto mess2 = rmq::RabbitMQConsumer::consume_message();
    EXPECT_TRUE(std::holds_alternative<nutc::messages::MarketOrder>(mess2));

    nutc::messages::MarketOrder actual2 = std::get<nutc::messages::MarketOrder>(mess2);
    ASSERT_EQ_MARKET_ORDER(
        actual2, "test_algos/buy_tsla_on_account", "APPL", nutc::messages::SIDE::BUY,
        100, 1
    );
}
