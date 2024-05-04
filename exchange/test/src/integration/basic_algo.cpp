#include "config.h"
#include "exchange/tickers/manager/ticker_manager.hpp"
#include "exchange/traders/trader_container.hpp"
#include "exchange/traders/trader_types/bot_trader.hpp"
#include "exchange/wrappers/messaging/consumer.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"
#include "test_utils/helpers/test_trader.hpp"
#include "test_utils/macros.hpp"
#include "test_utils/process.hpp"

#include <gtest/gtest.h>

namespace rmq = nutc::rabbitmq;
using nutc::util::Side::buy;
using nutc::util::Side::sell;

class IntegrationBasicAlgo : public ::testing::Test {
protected:
    using TestTrader = nutc::test_utils::TestTrader;

    void
    TearDown() override
    {
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
    auto traders = nutc::test_utils::initialize_testing_clients(users_, names);
    auto trader = traders.at(0);

    // want to see if it buys
    engine_manager_.add_engine("TSLA");

    auto bot = users_.add_trader<TestTrader>("TSLA", 0);
    bot->modify_holdings("TSLA", 1000); // NOLINT

    rmq::WrapperConsumer::match_new_order(
        engine_manager_, bot, nutc::messages::market_order{sell, "TSLA", 100, 100}
    );

    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    auto mess = nutc::test_utils::consume_message(trader);
    ASSERT_EQ_MARKET_ORDER(mess, "TSLA", buy, 100, 10);
}

TEST_F(IntegrationBasicAlgo, ManyUpdates)
{
    std::vector<std::string> names{"test_algos/confirm_1000.py"};
    auto traders = nutc::test_utils::initialize_testing_clients(users_, names);
    auto trader = traders.at(0);

    // want to see if it buys
    engine_manager_.add_engine("TSLA");

    auto bot = users_.add_trader<TestTrader>("TSLA", 0);
    bot->modify_holdings("TSLA", 100000); // NOLINT

    for (double i = 0; i < 100000; i++) {
        rmq::WrapperConsumer::match_new_order(
            engine_manager_, bot, nutc::messages::market_order{sell, "TSLA", 1, i}
        );
    }

    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    auto mess = nutc::test_utils::consume_message(trader);
    ASSERT_EQ_MARKET_ORDER(mess, "TSLA", buy, 100, 10);
}

TEST_F(IntegrationBasicAlgo, OnTradeUpdate)
{
    std::vector<std::string> names{"test_algos/buy_tsla_on_trade.py"};
    auto traders = nutc::test_utils::initialize_testing_clients(users_, names);
    auto trader = traders.at(0);

    engine_manager_.add_engine("TSLA");
    engine_manager_.add_engine("APPL");

    auto bot = users_.add_trader<TestTrader>("TSLA", 0);
    bot->modify_holdings("TSLA", 1000); // NOLINT

    rmq::WrapperConsumer::match_new_order(
        engine_manager_, bot, nutc::messages::market_order{sell, "TSLA", 100, 100}
    ); // NOLINT
    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    // obupdate triggers one user to place autil::Side::buy order of 10 TSLA at 100
    auto mess1 = nutc::test_utils::consume_message(trader);
    ASSERT_EQ_MARKET_ORDER(mess1, "TSLA", buy, 102, 10);

    rmq::WrapperConsumer::match_new_order(engine_manager_, trader, std::move(mess1));
    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    // on_trade_match triggers one user to place autil::Side::buy order of 1 TSLA at 100
    auto mess2 = nutc::test_utils::consume_message(trader);
    ASSERT_EQ_MARKET_ORDER(mess2, "APPL", buy, 100, 1);
}

// Sanity check that it goes through the orderbook
TEST_F(IntegrationBasicAlgo, MultipleLevelOrder)
{
    std::vector<std::string> names{"test_algos/buy_tsla_at_100.py"};
    auto traders = nutc::test_utils::initialize_testing_clients(users_, names);
    auto trader = traders.at(0);

    engine_manager_.add_engine("TSLA");

    auto bot = users_.add_trader<TestTrader>("TSLA", 0);
    bot->modify_holdings("TSLA", 1000); // NOLINT

    rmq::WrapperConsumer::match_new_order(
        engine_manager_, bot, nutc::messages::market_order{sell, "TSLA", 5, 100}
    ); // NOLINT
    rmq::WrapperConsumer::match_new_order(
        engine_manager_, bot, nutc::messages::market_order{sell, "TSLA", 5, 95}
    ); // NOLINT
    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    auto mess1 = nutc::test_utils::consume_message(trader);
    ASSERT_EQ_MARKET_ORDER(mess1, "TSLA", buy, 100, 10);

    rmq::WrapperConsumer::match_new_order(engine_manager_, trader, std::move(mess1));
    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    ASSERT_EQ(trader->get_capital() - trader->get_initial_capital(), -975);
}

TEST_F(IntegrationBasicAlgo, OnAccountUpdateSell)
{
    std::vector<std::string> names{"test_algos/sell_tsla_on_account.py"};
    auto traders = nutc::test_utils::initialize_testing_clients(users_, names);
    auto trader = traders.at(0);
    trader->modify_holdings("TSLA", 1000);
    auto bot = users_.add_trader<TestTrader>("TSLA", 100000);

    engine_manager_.add_engine("TSLA");
    engine_manager_.add_engine("APPL");

    rmq::WrapperConsumer::match_new_order(
        engine_manager_, bot, nutc::messages::market_order{buy, "TSLA", 102, 102}
    ); // NOLINT
    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    // obupdate triggers one user to place autil::Side::buy order of 10 TSLA at 102
    auto mess1 = nutc::test_utils::consume_message(trader);
    ASSERT_EQ_MARKET_ORDER(mess1, "TSLA", sell, 100, 10);

    rmq::WrapperConsumer::match_new_order(engine_manager_, trader, std::move(mess1));
    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    // on_trade_match triggers one user to place autil::Side::buy order of 1 TSLA at 100
    auto mess2 = nutc::test_utils::consume_message(trader);
    ASSERT_EQ_MARKET_ORDER(mess2, "APPL", buy, 100, 1);
}

TEST_F(IntegrationBasicAlgo, OnAccountUpdateBuy)
{
    std::vector<std::string> names{"test_algos/buy_tsla_on_account.py"};
    auto traders = nutc::test_utils::initialize_testing_clients(users_, names);
    auto trader = traders.at(0);

    engine_manager_.add_engine("TSLA");
    engine_manager_.add_engine("APPL");

    auto bot = users_.add_trader<TestTrader>("TSLA", 0);
    bot->modify_holdings("TSLA", 1000); // NOLINT

    rmq::WrapperConsumer::match_new_order(
        engine_manager_, bot, nutc::messages::market_order{sell, "TSLA", 100, 100}
    ); // NOLINT
    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    // obupdate triggers one user to place autil::Side::buy order of 10 TSLA at 102
    auto mess1 = nutc::test_utils::consume_message(trader);
    ASSERT_EQ_MARKET_ORDER(mess1, "TSLA", buy, 102, 10);

    rmq::WrapperConsumer::match_new_order(engine_manager_, trader, std::move(mess1));
    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    // on_trade_match triggers one user to place autil::Side::buy order of 1 TSLA at 100
    auto mess2 = nutc::test_utils::consume_message(trader);
    ASSERT_EQ_MARKET_ORDER(mess2, "APPL", buy, 100, 1);
}

TEST_F(IntegrationBasicAlgo, AlgoStartDelay)
{
    std::vector<std::string> names{"test_algos/buy_tsla_at_100.py"};
    auto traders = nutc::test_utils::initialize_testing_clients(
        users_, names, TEST_CLIENT_WAIT_SECS
    );
    auto trader = traders.at(0);

    auto start = std::chrono::high_resolution_clock::now();

    engine_manager_.add_engine("TSLA");
    auto bot = users_.add_trader<TestTrader>("TSLA", 0);
    bot->modify_holdings("TSLA", 1000); // NOLINT

    rmq::WrapperConsumer::match_new_order(
        engine_manager_, bot, nutc::messages::market_order{sell, "TSLA", 100, 100}
    ); // NOLINT
    nutc::engine_manager::EngineManager::get_instance().on_tick(0);

    auto mess2 = nutc::test_utils::consume_message(trader);

    auto end = std::chrono::high_resolution_clock::now();
    const int64_t duration_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    const double wait_time_ms = TEST_CLIENT_WAIT_SECS * 1000;

    EXPECT_GE(duration_ms, wait_time_ms);
    EXPECT_LE(duration_ms, wait_time_ms + 1000);
}
