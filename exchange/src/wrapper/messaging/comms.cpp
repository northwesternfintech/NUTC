#include "comms.hpp"

#include "shared/messages_exchange_to_wrapper.hpp"
#include "wrapper/logging.hpp"
#include "wrapper/pywrapper/pywrapper.hpp"

#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <unistd.h>

#include <chrono>

#include <iostream>

namespace nutc {
namespace comms {

// todo: split into helpers

void
ExchangeProxy::main_event_loop(const std::string& uid)
{
    while (true) {
        std::variant<start_time, tick_update> data = consume_message();
        std::visit(
            [&](auto&& arg) { process_message(std::forward<decltype(arg)>(arg), uid); },
            std::move(data)
        );
    }
}

void
ExchangeProxy::process_message(auto&& message, const std::string& uid)
{
    using MessageType = std::decay_t<decltype(message)>;
    if constexpr (std::is_same_v<MessageType, tick_update>) {
        for (const auto& update : message.ob_updates)
            handle_orderbook_update(update);
        for (const auto& update : message.matches)
            handle_match(update, uid);
    }
}

void
ExchangeProxy::handle_orderbook_update(const orderbook_update& update)
{
    try {
        std::string side = update.side == util::Side::buy ? "BUY" : "SELL";
        nutc::pywrapper::get_ob_update_function()(
            update.ticker, side, update.price, update.quantity
        );
    } catch (const py::error_already_set& e) {}
}

void
ExchangeProxy::handle_match(const match& match, const std::string& uid)
{
    try {
        std::string side = match.side == util::Side::buy ? "BUY" : "SELL";

        nutc::pywrapper::get_trade_update_function()(
            match.ticker, side, match.price, match.quantity
        );

        if (match.buyer_id == uid) {
            nutc::pywrapper::get_account_update_function()(
                match.ticker, "BUY", match.price, match.quantity, match.buyer_capital
            );
        }
        if (match.seller_id == uid) {
            nutc::pywrapper::get_account_update_function()(
                match.ticker, "SELL", match.price, match.quantity, match.seller_capital
            );
        }
    } catch (const py::error_already_set& e) {}
}

bool
ExchangeProxy::publish_market_order(
    const std::string& side, const std::string& ticker, double quantity, double price
)
{
    if (limiter.should_rate_limit()) {
        return false;
    }
    market_order order{
        side == "BUY" ? util::Side::buy : util::Side::sell, ticker, quantity, price
    };
    std::string message = glz::write_json(order);

    publish_message(message);
    return true;
}

void
ExchangeProxy::publish_message(const std::string& message)
{
    static constinit std::mutex lock{};
    lock.lock();
    std::cout << message << std::endl;
    lock.unlock();
}

std::variant<start_time, tick_update>
ExchangeProxy::consume_message()
{
    std::string buf{};
    std::getline(std::cin, buf);
    if (buf.empty())
        throw std::runtime_error("Wrapper received empty buffer from stdin");

    std::variant<start_time, tick_update> data{};
    auto err = glz::read_json(data, buf);
    if (err) {
        std::string error = glz::format_error(err, buf);
        throw std::runtime_error(
            fmt::format("Failed to parse message with error: {}", error)
        );
    }
    return data;
}

std::function<bool(const std::string&, const std::string&, double, double)>
ExchangeProxy::market_order_func()
{
    return [&](const std::string& side, const auto& ticker, const auto& quantity,
               const auto& price) {
        return ExchangeProxy::publish_market_order(side, ticker, quantity, price);
    };
}

void
ExchangeProxy::publish_init_message(bool ready)
{
    std::string message = glz::write_json(init_message{ready});
    publish_message(message);
}

// If wait_blocking is disabled, we block until we *receive* the message, but not
// after Otherwise, we block until the start time
void
ExchangeProxy::wait_for_start_time()
{
    using nanoseconds = std::chrono::nanoseconds;
    using time_point = std::chrono::high_resolution_clock::time_point;
    auto message = consume_message();

    // Sandbox may get ob updates before it's initialized
    while (!std::holds_alternative<start_time>(message)) {
        message = consume_message();
    }

    start_time start = std::get<start_time>(message);

    time_point wait_until{nanoseconds{start.start_time_ns}};
    std::this_thread::sleep_until(wait_until);
}

} // namespace comms
} // namespace nutc
