#include "process.hpp"

#include "exchange/algos/dev_mode/dev_mode.hpp"
#include "exchange/rabbitmq/client_manager/RabbitMQClientManager.hpp"

#include <csignal>

namespace nutc {
namespace testing_utils {

void
kill_all_processes(const manager::ClientManager& users)
{
    for (const auto& [_, client] : users.get_clients()) {
        kill(client.pid, SIGKILL);
    }
}

void
initialize_testing_clients(
    nutc::manager::ClientManager& users, const std::vector<std::string>& algo_filenames,
    client::SpawnMode mode
)
{
    using algo_mgmt::DevModeAlgoManager;
    using nutc::client::SpawnMode;

    DevModeAlgoManager algo_manager =
        DevModeAlgoManager(algo_filenames.size(), algo_filenames);
    algo_manager.initialize_client_manager(users);
    size_t num_users = spawn_all_clients(users, mode);
    rabbitmq::RabbitMQClientManager::wait_for_clients(users, num_users);
    rabbitmq::RabbitMQClientManager::send_start_time(users, CLIENT_WAIT_SECS);
}
} // namespace testing_utils
} // namespace nutc
