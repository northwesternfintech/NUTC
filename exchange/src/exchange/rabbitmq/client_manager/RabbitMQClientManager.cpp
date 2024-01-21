#include "RabbitMQClientManager.hpp"

#include "exchange/client_manager/client_manager.hpp"
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
                manager.set_active(message.client_id);
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

    auto send_to_client = [buf](const std::pair<std::string, manager::client_t>& pair) {
        const auto& [id, client] = pair;

        if (!client.active)
            return;

        RabbitMQPublisher::publish_message(id, buf);
    };

    const auto& clients = manager.get_clients();
    std::for_each(clients.begin(), clients.end(), send_to_client);
}

} // namespace rabbitmq
} // namespace nutc
