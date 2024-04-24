#include "consumer.hpp"

#include "exchange/concurrency/exchange_lock.hpp"
#include "exchange/tick_scheduler/tick_scheduler.hpp"
#include "exchange/traders/trader_container.hpp"
#include "exchange/wrappers/messaging/async_pipe_receiver.hpp"

#include <utility>

namespace nutc {
namespace rabbitmq {

template <typename T>
concept ExchangeReceivableMessage =
    std::is_same_v<std::decay_t<T>, messages::init_message>
    || std::is_same_v<std::decay_t<T>, messages::market_order>;

void
RabbitMQConsumer::consumer_event_loop(engine_manager::EngineManager& engine_manager)
{
    while (true) {
        concurrency::ExchangeLock::lock();
        auto incoming_message = consume_message_nonblocking();
        if (!incoming_message.has_value()) {
            concurrency::ExchangeLock::unlock();
            continue;
        }

        auto handle_message = [&]<typename t>(t&& arg)
        requires ExchangeReceivableMessage<t>
        {
            if constexpr (std::is_same_v<t, messages::init_message>) {
                throw std::runtime_error("Not expecting initialization message");
            }
            if constexpr (std::is_same_v<t, messages::market_order>) {
                match_new_order(engine_manager, std::forward<t>(arg));
            }
        };

        std::visit(handle_message, std::move(incoming_message.value()));
        concurrency::ExchangeLock::unlock();
    }
}

void
RabbitMQConsumer::match_new_order(
    engine_manager::EngineManager& engine_manager, messages::market_order&& order
)
{
    if (order.price < 0 || order.quantity <= 0)
        return;
    if (!engine_manager.has_engine(order.ticker))
        return;

    auto current_tick = ticks::TickJobScheduler::get().get_current_tick();
    auto trader = traders::TraderContainer::get_instance().get_trader(order.client_id);
    auto stored_order =
        matching::stored_order{trader,         order.side,  order.ticker,
                               order.quantity, order.price, current_tick};

    engine_manager.match_order(stored_order);
}

std::optional<std::string>
RabbitMQConsumer::consume_message_as_string()
{
    return wrappers::AsyncPipeReceiver::get().get_message();
}

std::optional<std::variant<messages::init_message, messages::market_order>>
RabbitMQConsumer::consume_message_nonblocking()
{
    std::optional<std::string> buf = consume_message_as_string();
    if (!buf.has_value()) {
        return std::nullopt;
    }

    std::variant<messages::init_message, messages::market_order> data;
    auto err = glz::read_json(data, buf.value());
    if (err) {
        std::string error = glz::format_error(err, buf.value());
        return std::nullopt;
    }
    return data;
}

} // namespace rabbitmq
} // namespace nutc
