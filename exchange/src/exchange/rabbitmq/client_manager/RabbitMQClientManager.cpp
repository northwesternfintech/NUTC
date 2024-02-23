#include "RabbitMQClientManager.hpp"

#include "exchange/logging.hpp"
#include "exchange/rabbitmq/consumer/RabbitMQConsumer.hpp"
#include "exchange/rabbitmq/publisher/RabbitMQPublisher.hpp"

namespace nutc {
namespace rabbitmq {

void
RabbitMQClientManager::wait_for_clients(
    manager::ClientManager& manager, size_t num_clients
)
{
    log_i(rabbitmq, "Blocking until all {} clients are ready to start...", num_clients);
    int num_running = 0;

    auto process_message = [&](const auto& message) {
        using t = std::decay_t<decltype(message)>;
        if constexpr (std::is_same_v<t, messages::MarketOrder>) {
            log_i(
                rabbitmq,
                "Received market order before initialization complete. Ignoring..."
            );
        }
        else if constexpr (std::is_same_v<t, messages::InitMessage>) {
            log_i(
                rabbitmq, "Received init message from client {} with status {}",
                message.client_id, message.ready ? "ready" : "not ready"
            );
            if (message.ready) {
                manager.get_trader(message.client_id)->set_active(/*active=*/true);
                num_running++;
            }
        }
        return true; // indicate that function should continue
    };

    for (size_t i = 0; i < num_clients; i++) {
        auto data = RabbitMQConsumer::consume_message();
        if (!std::visit(process_message, data)) {
            return;
        }
    }

    log_i(
        rabbitmq, "All {} clients ready. Starting exchange with {} ready clients",
        num_clients, num_running
    );
}

void
RabbitMQClientManager::send_start_time(
    const manager::ClientManager& manager, int wait_seconds
)
{
    using time_point = std::chrono::high_resolution_clock::time_point;
    time_point time =
        std::chrono::high_resolution_clock::now() + std::chrono::seconds(wait_seconds);
    int64_t time_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(time)
                          .time_since_epoch()
                          .count();

    messages::StartTime message{time_ns};
    std::string buf = glz::write_json(message);

    const auto& traders = manager.get_traders();
    for (const auto& [id, trader] : traders) {
        if (trader->is_active())
            RabbitMQPublisher::publish_message(id, buf);
    }
}

} // namespace rabbitmq
} // namespace nutc
