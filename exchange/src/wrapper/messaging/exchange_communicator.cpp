#include "exchange_communicator.hpp"

#include "common/messages_exchange_to_wrapper.hpp"
#include "common/messages_wrapper_to_exchange.hpp"
#include "common/types/position.hpp"
#include "common/util.hpp"
#include "wrapper/pywrapper/pywrapper.hpp"

#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <unistd.h>

#include <chrono>

#include <iostream>
#include <stdexcept>
#include <type_traits>

namespace nutc::wrapper {

using start_tick_variant_t = std::variant<start_time, tick_update>;

template <>
void
ExchangeCommunicator::process_message(start_time&)
{}

template <>
void
ExchangeCommunicator::process_message(tick_update& tick_update)
{
    std::ranges::for_each(
        tick_update.ob_updates,
        [&](const common::position& ob_update) { handle_orderbook_update(ob_update); }
    );
    std::ranges::for_each(tick_update.matches, [&](const match& match) {
        handle_match(match);
    });
}

void
ExchangeCommunicator::main_event_loop()
{
    while (true) {
        auto data = consume_message<start_tick_variant_t>();
        std::visit([this](auto message) { process_message(message); }, std::move(data));
    }
}

void
ExchangeCommunicator::handle_orderbook_update(const common::position& position)
{
    on_orderbook_update(position);
}

void
ExchangeCommunicator::handle_match(const match& match)
{
    on_trade_update(match.position);

    if (match.buyer_id == TRADER_ID) {
        on_account_update(match.position, match.buyer_capital);
    }
    if (match.seller_id == TRADER_ID) {
        on_account_update(match.position, match.seller_capital);
    }
}

template <typename T, typename... Args>
requires std::is_constructible_v<T, Args...>
bool
ExchangeCommunicator::publish_message(Args&&... args)
{
    if (limiter.should_rate_limit()) {
        return false;
    }
    auto message = glz::write_json(T{std::forward<Args>(args)...});
    if (!message.has_value()) [[unlikely]]
        throw std::runtime_error(glz::format_error(message.error()));

    publish_message(*message);
    return true;
}

void
ExchangeCommunicator::publish_message(const std::string& message)
{
    static constinit std::mutex lock{};
    lock.lock();
    std::cout << message << std::endl;
    lock.unlock();
}

common::algorithm_content
ExchangeCommunicator::consume_algorithm()
{
    return consume_message<algorithm_content>();
}

template <typename T>
T
ExchangeCommunicator::consume_message()
{
    std::string buf{};
    std::getline(std::cin, buf);
    if (buf.empty())
        throw std::runtime_error("Wrapper received empty buffer from stdin");

    T data{};
    auto err = glz::read_json(data, buf);
    if (err) {
        std::string error = glz::format_error(err, buf);
        throw std::runtime_error(
            fmt::format("Failed to parse message with error: {}", error)
        );
    }
    return data;
}

LimitOrderFunction
ExchangeCommunicator::place_limit_order()
{
    return [this](
               const std::string& side, const std::string& ticker, double quantity,
               double price, bool ioc
           ) {
        std::optional<common::Ticker> ticker_obj = to_ticker(ticker);
        if (!ticker_obj)
            throw std::runtime_error("uhh");
        common::Side side_enum =
            (side == "BUY") ? common::Side::buy : common::Side::sell;

        return publish_message<limit_order>(
            ticker_obj.value(), side_enum, quantity, price, ioc
        );
    };
}

// TODO: check for valid side/ticker
MarketOrderFunction
ExchangeCommunicator::place_market_order()
{
    return [this](const std::string& side, const std::string& ticker, double quantity) {
        std::optional<common::Ticker> ticker_obj = to_ticker(ticker);
        if (!ticker_obj)
            throw std::runtime_error("uhh");
        // return false;
        common::Side side_enum =
            (side == "BUY") ? common::Side::buy : common::Side::sell;

        return publish_message<market_order>(ticker_obj.value(), side_enum, quantity);
    };
}

bool
ExchangeCommunicator::report_startup_complete()
{
    return publish_message<common::init_message>();
}

// If wait_blocking is disabled, we block until we *receive* the message, but not
// after Otherwise, we block until the start time
void
ExchangeCommunicator::wait_for_start_time()
{
    using nanoseconds = std::chrono::nanoseconds;
    using time_point = std::chrono::high_resolution_clock::time_point;
    auto message = consume_message<start_tick_variant_t>();

    // Sandbox may get ob updates before it's initialized
    while (!std::holds_alternative<start_time>(message)) {
        message = consume_message<start_tick_variant_t>();
    }

    start_time start = std::get<start_time>(message);

    time_point wait_until{nanoseconds{start.start_time_ns}};
    std::this_thread::sleep_until(wait_until);
}

} // namespace nutc::wrapper
