#pragma once

#include "common/messages_exchange_to_wrapper.hpp"
#include "wrapper/pywrapper/pywrapper.hpp"
#include "wrapper/pywrapper/rate_limiter.hpp"

#include <unistd.h>

#include <string>

namespace nutc::wrapper {

using namespace nutc::common;

class ExchangeCommunicator {
    RateLimiter limiter{};
    const std::string TRADER_ID;
    OrderBookUpdateFunction on_orderbook_update;
    TradeUpdateFunction on_trade_update;
    AccountUpdateFunction on_account_update;

public:
    ExchangeCommunicator(
        std::string trader_id, OrderBookUpdateFunction ob_update,
        TradeUpdateFunction trade_update, AccountUpdateFunction account_update
    ) :
        TRADER_ID(std::move(trader_id)), on_orderbook_update{std::move(ob_update)},
        on_trade_update{std::move(trade_update)},
        on_account_update{std::move(account_update)}
    {}

    bool report_startup_complete();

    LimitOrderFunction place_limit_order();

    MarketOrderFunction place_market_order();

    void wait_for_start_time();

    void main_event_loop();

    common::algorithm_content consume_algorithm();

private:
    void handle_orderbook_update(const common::position& update);
    void handle_match(const match& match);

    template <typename T>
    void process_message(T&& message);

    static void publish_message(const std::string& message);

    template <typename T, typename... Args>
    requires std::is_constructible_v<T, Args...>
    [[nodiscard]] bool publish_message(Args&&...);

    template <typename T>
    T consume_message();
};

} // namespace nutc::wrapper
