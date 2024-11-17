#include "common/util.hpp"
#include "linter/spawning/spawning.hpp"

#include <gtest/gtest.h>

// TODO: put these in files, similar to the normal integration tests
// Then we can parameterize
namespace {
constexpr auto BASIC_ALGO = R"(class Strategy:
    def __init__(self) -> None:
        pass

    def on_trade_update(self, ticker: Ticker, side: Side, price: float, quantity: float) -> None:
        pass

    def on_orderbook_update(
        self, ticker: Ticker, side: Side, price: float, quantity: float
    ) -> None:
        pass

    def on_account_update(
        self,
        ticker: Ticker,
        side: Side,
        price: float,
        quantity: float,
        capital_remaining: float,
    ) -> None:
        pass
)";

constexpr auto timeout_algo = R"(import time
class Strategy:
    def __init__(self) -> None:
        time.sleep(20)

    def on_trade_update(self, ticker: Ticker, side: Side, price: float, quantity: float) -> None:
        pass

    def on_orderbook_update(
        self, ticker: Ticker, side: Side, price: float, quantity: float
    ) -> None:
        pass

    def on_account_update(
        self,
        ticker: Ticker,
        side: Side,
        price: float,
        quantity: float,
        capital_remaining: float,
    ) -> None:
        pass
)";

constexpr auto MISSING_ON_TRADE_UPDATE_ALGO = R"(class Strategy:
    def __init__(self) -> None:
        pass
    def on_orderbook_update(
        self, ticker: Ticker, side: Side, price: float, quantity: float
    ) -> None:
        pass

    def on_account_update(
        self,
        ticker: Ticker,
        side: Side,
        price: float,
        quantity: float,
        capital_remaining: float,
    ) -> None:
        pass
)";

constexpr auto MISSING_ON_ORDERBOOK_UPDATE_ALGO = R"(class Strategy:
    def __init__(self) -> None:
        pass

    def on_trade_update(self, ticker: Ticker, side: Side, price: float, quantity: float) -> None:
        pass
    def on_account_update(
        self,
        ticker: Ticker,
        side: Side,
        price: float,
        quantity: float,
        capital_remaining: float,
    ) -> None:
        pass
)";

constexpr auto MISSING_ON_ACCOUNT_UPDATE_ALGO = R"(class Strategy:
    def __init__(self) -> None:
        pass
        
    def on_trade_update(self, ticker: Ticker, side: Side, price: float, quantity: float) -> None:
        pass
    def on_orderbook_update(
        self, ticker: Ticker, side: Side, price: float, quantity: float
    ) -> None:
        pass
)";

using nutc::common::AlgoLanguage;
using nutc::linter::spawn_client;
} // namespace

namespace nutc::test {
TEST(IntegrationLinterPyTest, basic)
{
    auto lint_result = spawn_client(BASIC_ALGO, AlgoLanguage::python);
    ASSERT_TRUE(lint_result.success);
}

TEST(IntegrationLinterPyTest, timeout)
{
    std::chrono::milliseconds timeout{100};
    auto lint_result = spawn_client(timeout_algo, AlgoLanguage::python, timeout);
    ASSERT_FALSE(lint_result.success);
    EXPECT_TRUE(
        lint_result.message.find("Your code did not execute within")
        != std::string::npos
    );
}

TEST(IntegrationLinterPyTest, invalidAlgo)
{
    std::string algo = R"(not_valid_python)";
    auto lint_result = spawn_client(algo, AlgoLanguage::python);
    ASSERT_FALSE(lint_result.success);
    ASSERT_TRUE(
        lint_result.message.find("Failed to import code:") != std::string::npos
    );
}

TEST(IntegrationLinterPyTest, noStrategyClass)
{
    std::string algo = R"(import math)";
    auto lint_result = spawn_client(algo, AlgoLanguage::python);
    ASSERT_FALSE(lint_result.success);
    ASSERT_TRUE(
        lint_result.message.find("NameError: name 'Strategy' is not defined")
        != std::string::npos
    );
}

TEST(IntegrationLinterPyTest, MissingOnTradeUpdateFunction)
{
    auto lint_result = spawn_client(MISSING_ON_TRADE_UPDATE_ALGO, AlgoLanguage::python);
    ASSERT_FALSE(lint_result.success);
    EXPECT_TRUE(
        lint_result.message.find("has no attribute 'on_trade_update'")
        != std::string::npos
    );
}

TEST(IntegrationLinterPyTest, MissingOnOrderBookUpdateFunction)
{
    auto lint_result =
        spawn_client(MISSING_ON_ORDERBOOK_UPDATE_ALGO, AlgoLanguage::python);
    ASSERT_FALSE(lint_result.success);
    EXPECT_TRUE(
        lint_result.message.find("has no attribute 'on_orderbook_update'")
        != std::string::npos
    );
}

TEST(IntegrationLinterPyTest, MissingOnAccountUpdatefunction)
{
    auto lint_result =
        spawn_client(MISSING_ON_ACCOUNT_UPDATE_ALGO, AlgoLanguage::python);
    ASSERT_FALSE(lint_result.success);
    EXPECT_TRUE(
        lint_result.message.find("has no attribute 'on_account_update'")
        != std::string::npos
    );
}
} // namespace nutc::test
