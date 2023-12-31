#include "process.hpp"

#include "local_algos/dev_mode.hpp"
#include "process_spawning/spawning.hpp"
#include "rabbitmq/client_manager/RabbitMQClientManager.hpp"

#include <signal.h>

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
    nutc::manager::ClientManager& users, const std::vector<std::string>& algo_filenames
)
{
    dev_mode::init_client_manager_from_filenames(users, algo_filenames);
    size_t num_users = nutc::client::spawn_all_clients(users);
    rabbitmq::RabbitMQClientManager::wait_for_clients(users, num_users);
    rabbitmq::RabbitMQClientManager::send_start_time(users, CLIENT_WAIT_SECS);
}
} // namespace testing_utils
} // namespace nutc
