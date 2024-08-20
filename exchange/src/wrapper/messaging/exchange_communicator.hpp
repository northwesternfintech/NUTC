#pragma once

#include "shared/messages_exchange_to_wrapper.hpp"
#include "wrapper/pywrapper/pywrapper.hpp"
#include "wrapper/pywrapper/rate_limiter.hpp"

#include <unistd.h>

#include <string>

namespace nutc {

namespace messaging {
using namespace nutc::messages;

class ExchangeCommunicator {
    rate_limiter::RateLimiter limiter{};
    const std::string TRADER_ID;
    pywrapper::OrderBookUpdateFunction on_orderbook_update;
    pywrapper::TradeUpdateFunction on_trade_update;
    pywrapper::AccountUpdateFunction on_account_update;

public:
    ExchangeCommunicator(
        std::string trader_id, pywrapper::OrderBookUpdateFunction ob_update,
        pywrapper::TradeUpdateFunction trade_update,
        pywrapper::AccountUpdateFunction account_update
    ) :
        TRADER_ID(std::move(trader_id)), on_orderbook_update{ob_update},
        on_trade_update{trade_update}, on_account_update{account_update}
    {}

    bool report_startup_complete();

    pywrapper::LimitOrderFunction place_limit_order();

    pywrapper::MarketOrderFunction place_market_order();

    void wait_for_start_time();

    void main_event_loop();

    messages::algorithm_content consume_algorithm();

private:
    void handle_orderbook_update(const util::position& update);
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

} // namespace messaging
} // namespace nutc
