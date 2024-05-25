#include "spawning/spawning.hpp"

#include <gtest/gtest.h>

class IntegrationLinterTest : public ::testing::Test {
protected:
    nutc::spawning::LintProcessManager manager;
};

const std::string basic_algo = R"(class Strategy:
    def __init__(self) -> None:
        pass

    def on_trade_update(self, ticker: str, side: str, price: float, quantity: float) -> None:
        pass

    def on_orderbook_update(
        self, ticker: str, side: str, price: float, quantity: float
    ) -> None:
        pass

    def on_account_update(
        self,
        ticker: str,
        side: str,
        price: float,
        quantity: float,
        capital_remaining: float,
    ) -> None:
        pass
)";

const std::string timeout_algo = R"(import time
class Strategy:
    def __init__(self) -> None:
        time.sleep(20)

    def on_trade_update(self, ticker: str, side: str, price: float, quantity: float) -> None:
        pass

    def on_orderbook_update(
        self, ticker: str, side: str, price: float, quantity: float
    ) -> None:
        pass

    def on_account_update(
        self,
        ticker: str,
        side: str,
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
        self, ticker: str, side: str, price: float, quantity: float
    ) -> None:
        pass

    def on_account_update(
        self,
        ticker: str,
        side: str,
        price: float,
        quantity: float,
        capital_remaining: float,
    ) -> None:
        pass
)";

const std::string missing_on_orderbook_update_algo = R"(class Strategy:
    def __init__(self) -> None:
        pass

    def on_trade_update(self, ticker: str, side: str, price: float, quantity: float) -> None:
        pass
    def on_account_update(
        self,
        ticker: str,
        side: str,
        price: float,
        quantity: float,
        capital_remaining: float,
    ) -> None:
        pass
)";

const std::string missing_on_account_update_algo = R"(class Strategy:
    def __init__(self) -> None:
        pass
        
    def on_trade_update(self, ticker: str, side: str, price: float, quantity: float) -> None:
        pass
    def on_orderbook_update(
        self, ticker: str, side: str, price: float, quantity: float
    ) -> None:
        pass
)";

TEST_F(IntegrationLinterTest, basic)
{
    auto lint_result = manager.spawn_client(basic_algo);
    ASSERT_TRUE(lint_result.success);
}

TEST_F(IntegrationLinterTest, timeout)
{
    auto lint_result = manager.spawn_client(timeout_algo);
    ASSERT_FALSE(lint_result.success);
    ASSERT_TRUE(
        lint_result.message.find("Your code did not execute within")
        != std::string::npos
    );
}

TEST_F(IntegrationLinterTest, invalidAlgo)
{
    std::string algo = R"(not_valid_python)";
    auto lint_result = manager.spawn_client(algo);
    ASSERT_FALSE(lint_result.success);
    ASSERT_TRUE(
        lint_result.message.find("Failed to import code:") != std::string::npos
    );
}

TEST_F(IntegrationLinterTest, noStrategyClass)
{
    std::string algo = R"(import math)";
    auto lint_result = manager.spawn_client(algo);
    ASSERT_FALSE(lint_result.success);
    ASSERT_TRUE(
        lint_result.message.find("NameError: name 'Strategy' is not defined")
        != std::string::npos
    );
}

TEST_F(IntegrationLinterTest, missingRequiredFunction)
{
    auto lint_result = manager.spawn_client(missing_on_trade_update_algo);
    ASSERT_FALSE(lint_result.success);
    ASSERT_TRUE(
        lint_result.message.find("has no attribute 'on_trade_update'")
        != std::string::npos
    );

    lint_result = manager.spawn_client(missing_on_orderbook_update_algo);
    ASSERT_FALSE(lint_result.success);
    ASSERT_TRUE(
        lint_result.message.find("has no attribute 'on_orderbook_update'")
        != std::string::npos
    );

    lint_result = manager.spawn_client(missing_on_account_update_algo);
    ASSERT_FALSE(lint_result.success);
    ASSERT_TRUE(
        lint_result.message.find("has no attribute 'on_account_update'")
        != std::string::npos
    );
}
