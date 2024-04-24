#include "consumer.hpp"

#include "exchange/concurrency/exchange_lock.hpp"
#include "exchange/logging.hpp"
#include "exchange/tick_scheduler/tick_scheduler.hpp"
#include "exchange/traders/trader_container.hpp"
#include "exchange/wrappers/messaging/async_pipe_runner.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"

#include <utility>

namespace nutc {
namespace rabbitmq {

template <typename T>
concept ExchangeReceivableMessage =
    std::is_same_v<std::decay_t<T>, messages::init_message>
    || std::is_same_v<std::decay_t<T>, messages::market_order>;

void
WrapperConsumer::consumer_event_loop(engine_manager::EngineManager& engine_manager)
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
                handle_sandbox_init(std::forward<t>(arg));
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
WrapperConsumer::handle_sandbox_init(messages::init_message&& message)
{
    auto& trader_container = traders::TraderContainer::get_instance();
    if (!message.ready) {
        log_i(
            main, "Sandbox wrapper with id {} reported bad initialization",
            message.client_id
        );
        trader_container.get_instance().remove_trader(message.client_id);
        return;
    }
    trader_container.get_instance()
        .get_trader(message.client_id)
        ->send_messages({glz::write_json(messages::start_time{0})});
    log_i(main, "Sandbox wrapper with id {} fully initialized", message.client_id);
}

void
WrapperConsumer::match_new_order(
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
WrapperConsumer::consume_message_as_string()
{
    return wrappers::AsyncPipeRunner::get().get_message();
}

std::optional<std::variant<messages::init_message, messages::market_order>>
WrapperConsumer::consume_message_nonblocking()
{
    std::optional<std::string> buf = consume_message_as_string();
    if (!buf.has_value()) {
        return std::nullopt;
    }

    std::variant<messages::init_message, messages::market_order> data;
    auto err = glz::read_json(data, buf.value());
    if (err) {
        std::string error = glz::format_error(err, buf.value());
        log_e(main, "{}", error);
        log_e(main, "{}", buf.value());
        return std::nullopt;
    }
    return data;
}

} // namespace rabbitmq
} // namespace nutc
