#pragma once

#include "common/messages_exchange_to_wrapper.hpp"
#include "rate_limiter.hpp"

#include <unistd.h>

#include <string>

namespace nutc::wrapper {

using namespace nutc::common;

using LimitOrderFunction = std::function<order_id_t(
    common::Side side, const std::string& ticker, double quantity, double price,
    bool ioc
)>;
using MarketOrderFunction =
    std::function<bool(common::Side side, const std::string& ticker, double quantity)>;
using CancelOrderFunction =
    std::function<bool(const std::string& ticker, order_id_t order_id)>;

class ExchangeCommunicator {
    RateLimiter limiter_{};
    std::string trader_id_;

public:
    ExchangeCommunicator(std::string trader_id) : trader_id_(std::move(trader_id)) {}

    bool report_startup_complete();

    void wait_for_start_time();

    algorithm_content consume_algorithm();

    template <typename T>
    T consume_message();

    static void publish_message(const std::string& message);

    [[nodiscard]] bool
    publish_message(const auto& message)
    {
        if (limiter_.should_rate_limit()) {
            return false;
        }
        auto message_opt = glz::write_json(message);
        if (!message_opt.has_value()) [[unlikely]]
            throw std::runtime_error(glz::format_error(message_opt.error()));

        publish_message(message_opt.value());
        return true;
    }

    LimitOrderFunction place_limit_order();
    MarketOrderFunction place_market_order();
    CancelOrderFunction cancel_order();
};

} // namespace nutc::wrapper
