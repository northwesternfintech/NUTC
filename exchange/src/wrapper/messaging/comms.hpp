#pragma once

#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"
#include "wrapper/pywrapper/rate_limiter.hpp"

#include <unistd.h>

#include <string>

using init_message = nutc::messages::init_message;
using market_order = nutc::messages::market_order;
using tick_update = nutc::messages::tick_update;
using orderbook_update = nutc::messages::orderbook_update;
using match = nutc::messages::match;
using start_time = nutc::messages::start_time;
using algorithm_t = nutc::messages::algorithm_content;

namespace nutc {

namespace comms {
class ExchangeProxy {
public:
    static void publish_init_message();

    std::function<bool(const std::string&, const std::string&, double, double)>
    market_order_func();

    static void wait_for_start_time();

    void main_event_loop(const std::string& uid);

private:
    rate_limiter::RateLimiter limiter;

    static void handle_orderbook_update(const orderbook_update& update);
    static void handle_match(const match& match, const std::string& uid);
    template <typename T>
    static void process_message(T&& message, const std::string& uid);

    static void publish_message(const std::string& message);
    [[nodiscard]] bool publish_market_order(
        const std::string& side, const std::string& ticker, double quantity,
        double price
    );

    static algorithm_t consume_algorithm();
    static std::variant<start_time, tick_update> consume_message();
};

} // namespace comms
} // namespace nutc
