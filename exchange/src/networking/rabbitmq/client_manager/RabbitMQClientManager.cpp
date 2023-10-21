#include "RabbitMQClientManager.hpp"

#include "client_manager/client_manager.hpp"
#include "logging.hpp"
#include "networking/rabbitmq/consumer/RabbitMQConsumer.hpp"
#include "networking/rabbitmq/publisher/RabbitMQPublisher.hpp"

namespace nutc {
namespace rabbitmq {

void
RabbitMQClientManager::waitForClients(
    manager::ClientManager& clients, const int num_clients
)
{
    int num_running = 0;

    auto processMessage = [&](const auto& message) {
        using T = std::decay_t<decltype(message)>;
        if constexpr (std::is_same_v<T, messages::RMQError>) {
            log_e(
                rabbitmq, "Failed to consume message with error {}.", message.message
            );
            return false;
        }
        else if constexpr (std::is_same_v<T, messages::MarketOrder>) {
            log_i(
                rabbitmq,
                "Received market order before initialization complete. Ignoring..."
            );
        }
        else if constexpr (std::is_same_v<T, messages::InitMessage>) {
            log_i(
                rabbitmq, "Received init message from client {} with status {}",
                message.client_uid, message.ready ? "ready" : "not ready"
            );
            if (message.ready) {
                clients.set_active(message.client_uid);
                num_running++;
            }
        }
        return true; // indicate that function should continue
    };

    for (int i = 0; i < num_clients; i++) {
        auto data = RabbitMQConsumer::consumeMessage();
        if (!std::visit(processMessage, data)) {
            return;
        }
    }

    log_i(
        rabbitmq, "All {} clients ready. Starting exchange with {} ready clients",
        num_clients, num_running
    );
}

void
RabbitMQClientManager::sendStartTime(
    const manager::ClientManager& manager, int wait_seconds
)
{
    std::vector<manager::Client> active_clients = manager.get_clients(true);
    using time_point = std::chrono::high_resolution_clock::time_point;
    time_point time =
        std::chrono::high_resolution_clock::now() + std::chrono::seconds(wait_seconds);
    long long time_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(time)
                            .time_since_epoch()
                            .count();

    messages::StartTime message{time_ns};
    std::string buf = glz::write_json(message);
    auto send_to_client = [buf](const manager::Client& client) {
        RabbitMQPublisher::publishMessage(client.uid, buf);
    };

    for (const auto& client : active_clients) {
        send_to_client(client);
    }
}

} // namespace rabbitmq
} // namespace nutc
