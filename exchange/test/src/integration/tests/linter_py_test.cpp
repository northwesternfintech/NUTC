#include "linter/spawning/spawning.hpp"

#include <gtest/gtest.h>

class IntegrationLinterPyTest : public ::testing::Test {
protected:
    nutc::spawning::LintProcessManager manager;
};

const std::string basic_algo = R"(class Strategy:
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

// TODO: fix/restore
const std::string timeout_algo = R"(import time
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

const std::string missing_on_trade_update_algo = R"(class Strategy:
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

const std::string missing_on_orderbook_update_algo = R"(class Strategy:
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

const std::string missing_on_account_update_algo = R"(class Strategy:
    def __init__(self) -> None:
        pass
        
    def on_trade_update(self, ticker: Ticker, side: Side, price: float, quantity: float) -> None:
        pass
    def on_orderbook_update(
        self, ticker: Ticker, side: Side, price: float, quantity: float
    ) -> None:
        pass
)";

TEST_F(IntegrationLinterPyTest, basic)
{
    auto lint_result =
        manager.spawn_client(basic_algo, nutc::spawning::AlgoLanguage::Python);
    ASSERT_TRUE(lint_result.success);
}

TEST_F(IntegrationLinterPyTest, timeout)
{
    // TODO: complete
    return;
    auto lint_result =
        manager.spawn_client(timeout_algo, nutc::spawning::AlgoLanguage::Python);
    ASSERT_FALSE(lint_result.success);
    ASSERT_TRUE(
        lint_result.message.find("Your code did not execute within")
        != std::string::npos
    );
}

TEST_F(IntegrationLinterPyTest, invalidAlgo)
{
    std::string algo = R"(not_valid_python)";
    auto lint_result = manager.spawn_client(algo, nutc::spawning::AlgoLanguage::Python);
    ASSERT_FALSE(lint_result.success);
    ASSERT_TRUE(
        lint_result.message.find("Failed to import code:") != std::string::npos
    );
}

TEST_F(IntegrationLinterPyTest, noStrategyClass)
{
    std::string algo = R"(import math)";
    auto lint_result = manager.spawn_client(algo, nutc::spawning::AlgoLanguage::Python);
    ASSERT_FALSE(lint_result.success);
    ASSERT_TRUE(
        lint_result.message.find("NameError: name 'Strategy' is not defined")
        != std::string::npos
    );
}

TEST_F(IntegrationLinterPyTest, missingRequiredFunction)
{
    auto lint_result = manager.spawn_client(
        missing_on_trade_update_algo, nutc::spawning::AlgoLanguage::Python
    );
    ASSERT_FALSE(lint_result.success);
    ASSERT_TRUE(
        lint_result.message.find("has no attribute 'on_trade_update'")
        != std::string::npos
    );

    lint_result = manager.spawn_client(
        missing_on_orderbook_update_algo, nutc::spawning::AlgoLanguage::Python
    );
    ASSERT_FALSE(lint_result.success);
    ASSERT_TRUE(
        lint_result.message.find("has no attribute 'on_orderbook_update'")
        != std::string::npos
    );

    lint_result = manager.spawn_client(
        missing_on_account_update_algo, nutc::spawning::AlgoLanguage::Python
    );
    ASSERT_FALSE(lint_result.success);
    ASSERT_TRUE(
        lint_result.message.find("has no attribute 'on_account_update'")
        != std::string::npos
    );
}
