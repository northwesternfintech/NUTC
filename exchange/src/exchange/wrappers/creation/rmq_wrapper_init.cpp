#include "rmq_wrapper_init.hpp"

#include "exchange/logging.hpp"
#include "exchange/wrappers/messaging/consumer.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"

namespace nutc {
namespace rabbitmq {

void
RabbitMQWrapperInitializer::wait_for_clients(traders::TraderContainer& manager)
{
    size_t num_clients = manager.num_traders();
    log_i(rabbitmq, "Blocking until all {} clients are ready to start...", num_clients);
    int num_running = 0;

    auto process_message = [&](const auto& message) {
        using t = std::decay_t<decltype(message)>;
        if constexpr (std::is_same_v<t, messages::market_order>) {
            log_i(
                rabbitmq,
                "Received market order before initialization complete. Ignoring..."
            );
        }
        else if constexpr (std::is_same_v<t, messages::init_message>) {
            log_i(
                rabbitmq, "Received init message from client {} with status {}",
                message.client_id, message.ready ? "ready" : "not ready"
            );

            // TODO: maybe send some warning?
            if (!message.ready) {
                manager.remove_trader(message.client_id);
                num_running++;
            }
            return true;
        }
        return false;
    };

    for (size_t i = 0; i < num_clients; i++) {
        auto data = RabbitMQConsumer::consume_message();
        while (!std::visit(process_message, data)) {}
    }

    log_i(
        rabbitmq, "All {} clients ready. Starting exchange with {} ready clients",
        num_clients, num_running
    );
}

void
RabbitMQWrapperInitializer::send_start_time(
    traders::TraderContainer& manager, size_t wait_seconds
)
{
    using time_point = std::chrono::high_resolution_clock::time_point;
    time_point time =
        std::chrono::high_resolution_clock::now() + std::chrono::seconds(wait_seconds);
    int64_t time_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(time)
                          .time_since_epoch()
                          .count();

    messages::start_time message{time_ns};
    std::string buf = glz::write_json(message);

    manager.broadcast_messages({buf});
}

} // namespace rabbitmq
} // namespace nutc
