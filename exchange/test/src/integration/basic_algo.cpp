#include "config.h"
#include "exchange/rabbitmq/connection_handler/rmq_connection_handler.hpp"
#include "exchange/rabbitmq/consumer/rmq_consumer.hpp"
#include "exchange/rabbitmq/order_handler/rmq_order_handler.hpp"
#include "exchange/tickers/manager/ticker_manager.hpp"
#include "exchange/traders/trader_types/bot_trader.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"
#include "test_utils/macros.hpp"
#include "test_utils/process.hpp"

#include <gtest/gtest.h>

namespace rmq = nutc::rabbitmq;

class IntegrationBasicAlgo : public ::testing::Test {
protected:
    using BotTrader = nutc::bots::BotTrader;

    void
    SetUp() override
    {
        auto& rmq_conn = rmq::RabbitMQConnectionManager::get_instance();
        bool connected = rmq_conn.initialize_connection();

        if (!connected) {
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

    TraderContainer& users_ =
        nutc::traders::TraderContainer::get_instance(); // NOLINT(*)
    nutc::engine_manager::EngineManager& engine_manager_ =
        nutc::engine_manager::EngineManager::get_instance(); // NOLINT(*)
};

TEST_F(IntegrationBasicAlgo, InitialLiquidity)
{
    std::vector<std::string> names{"test_algos/buy_tsla_at_100.py"};
    if (!nutc::testing_utils::initialize_testing_clients(users_, names)) {
        FAIL() << "Failed to initialize testing clients";
    }

    // want to see if it buys
    engine_manager_.add_engine("TSLA");

    auto bot = users_.add_trader<BotTrader>("", 0);
    bot->modify_holdings("TSLA", 1000); // NOLINT

    rmq::RabbitMQOrderHandler::handle_incoming_market_order(
        engine_manager_,
        nutc::messages::market_order{
            bot->get_id(), nutc::util::Side::sell, "TSLA", 100, 100
        }
    );

    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    auto mess = rmq::RabbitMQConsumer::consume_message();
    ASSERT_TRUE(std::holds_alternative<nutc::messages::market_order>(mess));

    nutc::messages::market_order actual = std::get<nutc::messages::market_order>(mess);
    ASSERT_EQ_MARKET_ORDER(
        actual, "test_algos/buy_tsla_at_100.py", "TSLA", nutc::util::Side::buy, 100, 10
    );
}

TEST_F(IntegrationBasicAlgo, OnTradeUpdate)
{
    std::vector<std::string> names{"test_algos/buy_tsla_on_trade.py"};
    if (!nutc::testing_utils::initialize_testing_clients(users_, names)) {
        FAIL() << "Failed to initialize testing clients";
    }

    engine_manager_.add_engine("TSLA");
    engine_manager_.add_engine("APPL");

    auto bot = users_.add_trader<BotTrader>("", 0);
    bot->modify_holdings("TSLA", 1000); // NOLINT

    rmq::RabbitMQOrderHandler::handle_incoming_market_order(
        engine_manager_,
        nutc::messages::market_order{
            bot->get_id(), nutc::util::Side::sell, "TSLA", 100, 100
        }
    ); // NOLINT
    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    // obupdate triggers one user to place autil::Side::buy order of 10 TSLA at 100
    auto mess1 = rmq::RabbitMQConsumer::consume_message();
    EXPECT_TRUE(std::holds_alternative<nutc::messages::market_order>(mess1));

    nutc::messages::market_order actual_mo =
        std::get<nutc::messages::market_order>(mess1);
    ASSERT_EQ_MARKET_ORDER(
        actual_mo, "test_algos/buy_tsla_on_trade.py", "TSLA", nutc::util::Side::buy,
        102, 10
    );

    rmq::RabbitMQOrderHandler::handle_incoming_market_order(
        engine_manager_, std::move(actual_mo)
    );
    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    // on_trade_match triggers one user to place autil::Side::buy order of 1 TSLA at 100
    auto mess2 = rmq::RabbitMQConsumer::consume_message();
    EXPECT_TRUE(std::holds_alternative<nutc::messages::market_order>(mess2));

    nutc::messages::market_order actual2 =
        std::get<nutc::messages::market_order>(mess2);
    ASSERT_EQ_MARKET_ORDER(
        actual2, "test_algos/buy_tsla_on_trade.py", "APPL", nutc::util::Side::buy, 100,
        1
    );
}

TEST_F(IntegrationBasicAlgo, OnAccountUpdate)
{
    std::vector<std::string> names{"test_algos/buy_tsla_on_account.py"};
    if (!nutc::testing_utils::initialize_testing_clients(users_, names)) {
        FAIL() << "Failed to initialize testing clients";
    }

    engine_manager_.add_engine("TSLA");
    engine_manager_.add_engine("APPL");

    auto bot = users_.add_trader<BotTrader>("", 0);
    bot->modify_holdings("TSLA", 1000); // NOLINT

    rmq::RabbitMQOrderHandler::handle_incoming_market_order(
        engine_manager_,
        nutc::messages::market_order{
            bot->get_id(), nutc::util::Side::sell, "TSLA", 100, 100
        }
    ); // NOLINT
    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    // obupdate triggers one user to place autil::Side::buy order of 10 TSLA at 102
    auto mess1 = rmq::RabbitMQConsumer::consume_message();
    EXPECT_TRUE(std::holds_alternative<nutc::messages::market_order>(mess1));

    nutc::messages::market_order actual_mo =
        std::get<nutc::messages::market_order>(mess1);
    ASSERT_EQ_MARKET_ORDER(
        actual_mo, "test_algos/buy_tsla_on_account.py", "TSLA", nutc::util::Side::buy,
        102, 10
    );

    rmq::RabbitMQOrderHandler::handle_incoming_market_order(
        engine_manager_, std::move(actual_mo)
    );
    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    // on_trade_match triggers one user to place autil::Side::buy order of 1 TSLA at 100
    auto mess2 = rmq::RabbitMQConsumer::consume_message();
    EXPECT_TRUE(std::holds_alternative<nutc::messages::market_order>(mess2));

    nutc::messages::market_order actual2 =
        std::get<nutc::messages::market_order>(mess2);
    ASSERT_EQ_MARKET_ORDER(
        actual2, "test_algos/buy_tsla_on_account.py", "APPL", nutc::util::Side::buy,
        100, 1
    );
}

TEST_F(IntegrationBasicAlgo, AlgoStartDelay)
{
    std::vector<std::string> names{"test_algos/buy_tsla_at_100.py"};
    if (!nutc::testing_utils::initialize_testing_clients(
            users_, names, /*has_delay=*/true
        )) {
        FAIL() << "Failed to initialize testing clients";
    }

    auto start = std::chrono::high_resolution_clock::now();

    engine_manager_.add_engine("TSLA");
    auto bot = users_.add_trader<BotTrader>("", 0);
    bot->modify_holdings("TSLA", 1000); // NOLINT

    rmq::RabbitMQOrderHandler::handle_incoming_market_order(
        engine_manager_,
        nutc::messages::market_order{
            bot->get_id(), nutc::util::Side::sell, "TSLA", 100, 100
        }
    ); // NOLINT
    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    auto mess = rmq::RabbitMQConsumer::consume_message();

    auto end = std::chrono::high_resolution_clock::now();
    const int64_t duration_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    const double wait_time_ms = TEST_CLIENT_WAIT_SECS * 1000;

    EXPECT_GE(duration_ms, wait_time_ms);
    EXPECT_LE(duration_ms, wait_time_ms + 1000);
}
