#include "common/util.hpp"
#include "linter/spawning/spawning.hpp"

#include <gtest/gtest.h>

namespace {
constexpr auto BASIC_ALGO = R"(#include <cstdint>
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

constexpr auto TIMEOUT_ALGO = R"(#include <cstdint>
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
                           float price) {while(true) {}}

  void on_account_update(Ticker ticker, Side side, float price, float quantity,
                         float capital_remaining) {cancel_order(Ticker::BTC, 5);}
};
)";

constexpr auto SYNTAX_ERROR = R"(#include <cstdint>
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

constexpr auto EXCEPTION_THROWN = R"(#include <cstdint>
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

using nutc::common::AlgoLanguage;
using nutc::linter::spawn_client;
} // namespace

TEST(IntegrationLinterCppTest, basic)
{
    auto lint_result = spawn_client(BASIC_ALGO, AlgoLanguage::cpp);
    ASSERT_TRUE(lint_result.success);
}

TEST(IntegrationLinterCppTest, TimeoutTriggersSuccessfullyAndFinishesExecution)
{
    std::chrono::milliseconds timeout{100};
    auto lint_result = spawn_client(TIMEOUT_ALGO, AlgoLanguage::cpp, timeout);
    ASSERT_FALSE(lint_result.success);
    EXPECT_TRUE(
        lint_result.message.find("Your code did not execute within")
        != std::string::npos
    );
}

TEST(IntegrationLinterCppTest, SyntaxErrorDetection)
{
    auto lint_result = spawn_client(SYNTAX_ERROR, AlgoLanguage::cpp);
    ASSERT_FALSE(lint_result.success);
    EXPECT_TRUE(lint_result.message.contains("‘SYNTAX’ was not declared in this scope")
    );
}

TEST(IntegrationLinterCppTest, RuntimeError)
{
    auto lint_result = spawn_client(EXCEPTION_THROWN, AlgoLanguage::cpp);
    ASSERT_FALSE(lint_result.success);
    EXPECT_TRUE(lint_result.message.contains("Failed to run on_orderbook_update"));
    EXPECT_TRUE(lint_result.message.contains("This is an error"));
}
