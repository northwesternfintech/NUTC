#include "linter/spawning/spawning.hpp"

#include <gtest/gtest.h>

#include <iostream>

class IntegrationLinterCppTest : public ::testing::Test {
protected:
    nutc::spawning::LintProcessManager manager;
};

const std::string basic_algo = R"(#include <cstdint>
#include <string>
enum class Side { buy = 0, sell = 1 };
enum class Ticker : std::uint8_t { ETH = 0, BTC = 1, LTC = 2 }; // NOLINT
bool place_market_order(Side side, Ticker ticker, float quantity);
std::int64_t place_limit_order(Side side, Ticker ticker, float quantity,
                               float price, bool ioc = false);
bool cancel_order(Ticker ticker, std::int64_t order_id);

class Strategy {
public:
  Strategy() {
	place_market_order(Side::buy, Ticker::ETH, 1.0);
  }

  void on_trade_update(Ticker ticker, Side side, float quantity, float price) {place_limit_order(Side::buy, Ticker::LTC, 1.0, 1.0);}
  void on_orderbook_update(Ticker ticker, Side side, float quantity,
                           float price) {place_market_order(Side::sell, Ticker::BTC, 1.0);}

  void on_account_update(Ticker ticker, Side side, float price, float quantity,
                         float capital_remaining) {cancel_order(Ticker::BTC, 5);}
};
)";

const std::string syntax_error = R"(#include <cstdint>
#include <string>
enum class Side { buy = 0, sell = 1 };
enum class Ticker : std::uint8_t { ETH = 0, BTC = 1, LTC = 2 }; // NOLINT
bool place_market_order(Side side, Ticker ticker, float quantity);
std::int64_t place_limit_order(Side side, Ticker ticker, float quantity,
                               float price, bool ioc = false);
bool cancel_order(Ticker ticker, std::int64_t order_id);

class Strategy {
public:
  Strategy() {
  }

  void on_trade_update(Ticker ticker, Side side, float quantity, float price) {}
  void on_orderbook_update(Ticker ticker, Side side, float quantity,
                           float price) { SYNTAX ERROR }

  void on_account_update(Ticker ticker, Side side, float price, float quantity,
                         float capital_remaining) {}
};
)";

const std::string exception_thrown = R"(#include <cstdint>
#include <string>
#include <stdexcept>
enum class Side { buy = 0, sell = 1 };
enum class Ticker : std::uint8_t { ETH = 0, BTC = 1, LTC = 2 }; // NOLINT
bool place_market_order(Side side, Ticker ticker, float quantity);
std::int64_t place_limit_order(Side side, Ticker ticker, float quantity,
                               float price, bool ioc = false);
bool cancel_order(Ticker ticker, std::int64_t order_id);

class Strategy {
public:
  Strategy() {
  }

  void on_trade_update(Ticker ticker, Side side, float quantity, float price) {}
  void on_orderbook_update(Ticker ticker, Side side, float quantity,
                           float price) { throw std::runtime_error("This is an error"); }

  void on_account_update(Ticker ticker, Side side, float price, float quantity,
                         float capital_remaining) {}
};
)";

using nutc::spawning::AlgoLanguage;

TEST_F(IntegrationLinterCppTest, basic)
{
    auto lint_result = manager.spawn_client(basic_algo, AlgoLanguage::Cpp);
    std::cout << "first " << lint_result.message << "\n";
    ASSERT_TRUE(lint_result.success);
}

TEST_F(IntegrationLinterCppTest, SyntaxErrorDetection)
{
    auto lint_result = manager.spawn_client(syntax_error, AlgoLanguage::Cpp);
    ASSERT_FALSE(lint_result.success);
    EXPECT_TRUE(lint_result.message.contains("‘SYNTAX’ was not declared in this scope")
    );
}

TEST_F(IntegrationLinterCppTest, RuntimeError)
{
    auto lint_result = manager.spawn_client(exception_thrown, AlgoLanguage::Cpp);
    ASSERT_FALSE(lint_result.success);
    EXPECT_TRUE(lint_result.message.contains("Failed to run on_orderbook_update"));
    EXPECT_TRUE(lint_result.message.contains("This is an error"));
}
