#include "exchange_communicator.hpp"

#include "common/types/messages/messages_exchange_to_wrapper.hpp"
#include "common/types/messages/messages_wrapper_to_exchange.hpp"
#include "common/util.hpp"

#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <unistd.h>

#include <chrono>

#include <iostream>
#include <stdexcept>

namespace nutc::wrapper {

void
ExchangeCommunicator::publish_message(const std::string& message)
{
    static constinit std::mutex lock{};
    lock.lock();
    std::cout << message << std::endl;
    lock.unlock();
}

algorithm_content
ExchangeCommunicator::consume_algorithm()
{
    auto algorithm = wait_and_consume_message<algorithm_content>();
    algorithm.algorithm_content_str = base64_decode(algorithm.algorithm_content_str);

    return algorithm;
}

std::variant<common::tick_update, common::account_update>
ExchangeCommunicator::consume_market_update()
{
    return consume_message<std::variant<common::tick_update, common::account_update>>();
}

template <typename T>
T
ExchangeCommunicator::wait_and_consume_message()
{
    using message_variant = std::variant<tick_update, account_update, T>;
    message_variant data;
    while (!std::holds_alternative<T>(data)) {
        data = consume_message<message_variant>();
    }
    return std::get<T>(data);
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
    // TODO(anyone): this disables NRVO. Change up?
    auto err = glz::read_json(data, buf);
    if (err) {
        std::string error = glz::format_error(err, buf);
        throw std::runtime_error(
            fmt::format("Failed to parse message with error: {}", error)
        );
    }
    return data;
}

template <typename T, typename... Args>
requires std::is_constructible_v<T, Args...>
T
ExchangeCommunicator::publish_message(Args... args)
{
    T message{args...};
    auto message_opt = glz::write_json(message);
    if (!message_opt.has_value()) [[unlikely]]
        throw std::runtime_error(glz::format_error(message_opt.error()));

    publish_message(message_opt.value());
    return message;
}

LimitOrderFunction
ExchangeCommunicator::place_limit_order()
{
    return [this](
               common::Side side, common::Ticker ticker, double quantity, double price,
               bool ioc
           ) -> order_id_t {
        if (limiter_.should_rate_limit()) {
            return -1;
        }
        return publish_message<limit_order>(ticker, side, quantity, price, ioc)
            .order_id;
    };
}

MarketOrderFunction
ExchangeCommunicator::place_market_order()
{
    return [this](common::Side side, common::Ticker ticker, double quantity) {
        if (limiter_.should_rate_limit()) {
            return false;
        }
        publish_message<market_order>(ticker, side, quantity);
        return true;
    };
}

CancelOrderFunction
ExchangeCommunicator::cancel_order()
{
    return [](common::Ticker ticker, order_id_t order_id) {
        publish_message<common::cancel_order>(ticker, order_id);
        return true;
    };
}

void
ExchangeCommunicator::report_startup_complete()
{
    publish_message<common::init_message>();
}

// If wait_blocking is disabled, we block until we *receive* the message, but not
// after Otherwise, we block until the start time
void
ExchangeCommunicator::wait_for_start_time()
{
    using nanoseconds = std::chrono::nanoseconds;
    using time_point = std::chrono::high_resolution_clock::time_point;
    auto start = wait_and_consume_message<start_time>();

    time_point wait_until{nanoseconds{start.start_time_ns}};
    std::this_thread::sleep_until(wait_until);
}

} // namespace nutc::wrapper
