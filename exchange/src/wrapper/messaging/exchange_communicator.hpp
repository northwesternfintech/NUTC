#pragma once

#include "common/types/messages/messages_exchange_to_wrapper.hpp"
#include "rate_limiter.hpp"

#include <glaze/json/write.hpp>
#include <unistd.h>

#include <string>

namespace nutc::wrapper {

using namespace nutc::common;

using LimitOrderFunction = std::function<order_id_t(
    common::Side side, common::Ticker ticker, float quantity, float price, bool ioc
)>;
using MarketOrderFunction =
    std::function<bool(common::Side side, common::Ticker ticker, float quantity)>;
using CancelOrderFunction =
    std::function<bool(common::Ticker ticker, order_id_t order_id)>;

// TODO: this class handles low-level communication (encoding, decoding, etc) and higher
// level abstractions like the messages themselves, waiting for algo, etc
// We should split this into two classes
class ExchangeCommunicator {
    RateLimiter limiter_;
    std::string trader_id_;

public:
    ExchangeCommunicator(std::string trader_id) : trader_id_(std::move(trader_id)) {}

    static void report_startup_complete();

    static void wait_for_start_time();

    static algorithm_content consume_algorithm();
    static std::variant<common::tick_update, common::account_update>
    consume_market_update();

    LimitOrderFunction place_limit_order();
    MarketOrderFunction place_market_order();
    static CancelOrderFunction cancel_order();

private:
    template <typename T>
    static T consume_message();

    template <typename T>
    static T wait_and_consume_message();

    // TODO: glaze concepts maybe?
    template <typename T, typename... Args>
    requires std::is_constructible_v<T, Args...>
    static T publish_message(Args... args);

    static void publish_message(const std::string& message);
};

} // namespace nutc::wrapper
