#pragma once

#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"
#include "shared/util.hpp"
#include "wrapper/pywrapper/rate_limiter.hpp"

#include <unistd.h>

#include <string>

namespace nutc {

namespace comms {
using init_message = nutc::messages::init_message;
using limit_order = nutc::messages::limit_order;
using tick_update = nutc::messages::tick_update;
using match = nutc::messages::match;
using start_time = nutc::messages::start_time;
using algorithm_t = nutc::messages::algorithm_content;

class ExchangeProxy {
public:
    static void publish_init_message();

    std::function<bool(const std::string&, const std::string&, double, double, bool)>
    limit_order_func();

    std::function<bool(const std::string&, const std::string&, double)>
    market_order_func();

    static void wait_for_start_time();

    void main_event_loop(const std::string& uid);

    static algorithm_t consume_algorithm();

private:
    rate_limiter::RateLimiter limiter;

    static void handle_orderbook_update(const util::position& update);
    static void handle_match(const match& match, const std::string& uid);
    template <typename T>
    static void process_message(T&& message, const std::string& uid);

    static void publish_message(const std::string& message);

    template <typename T>
    [[nodiscard]] bool publish_order(const T& order);

    [[nodiscard]] bool
    publish_market_order(util::Side side, util::Ticker ticker, double quantity);

    template <typename T>
    static T consume_message();
};

} // namespace comms
} // namespace nutc
