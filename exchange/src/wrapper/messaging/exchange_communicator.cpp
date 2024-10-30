#include "exchange_communicator.hpp"

#include "common/types/messages/glz_messages_exchange_to_wrapper.hpp"
#include "common/types/messages/glz_messages_wrapper_to_exchange.hpp"
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

using start_tick_variant_t = std::variant<start_time, tick_update>;

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
    auto algorithm = consume_message<algorithm_content>();
    algorithm.algorithm_content_str = base64_decode(algorithm.algorithm_content_str);

    return algorithm;
}

common::tick_update
ExchangeCommunicator::consume_tick_update()
{
    return consume_message<common::tick_update>();
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
               common::Side side, common::Ticker ticker, double quantity, double price,
               bool ioc
           ) -> order_id_t {
        limit_order order{ticker, side, quantity, price, ioc};
        if (!publish_message(order))
            return -1;
        return order.order_id;
    };
}

MarketOrderFunction
ExchangeCommunicator::place_market_order()
{
    return [this](common::Side side, common::Ticker ticker, double quantity) {
        market_order order{ticker, side, quantity};
        return publish_message(order);
    };
}

CancelOrderFunction
ExchangeCommunicator::cancel_order()
{
    return [this](common::Ticker ticker, order_id_t order_id) -> bool {
        return publish_message(common::cancel_order{ticker, order_id});
    };
}

bool
ExchangeCommunicator::report_startup_complete()
{
    return publish_message(common::init_message{});
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
