#include "process.hpp"

#include "exchange/algos/dev_mode/dev_mode.hpp"
#include "exchange/config.h"
#include "exchange/rabbitmq/client_manager/RabbitMQClientManager.hpp"
#include "exchange/traders/trader_types.hpp"

#include <csignal>

namespace nutc {
namespace testing_utils {

void
kill_all_processes(const manager::ClientManager& users)
{
    for (const auto& [_, client] : users.get_clients_const()) {
        pid_t pid = std::visit(
            [&](auto&& arg) {
                using t = std::decay_t<decltype(arg)>;
                if constexpr (!std::is_same_v<t, manager::bot_trader_t>) {
                    return arg.get_pid();
                }
                return -1;
            },
            client
        );
        kill(pid, SIGKILL);
    }
}

void
initialize_testing_clients(
    nutc::manager::ClientManager& users, const std::vector<std::string>& algo_filenames,
    bool has_delay
)
{
    using algo_mgmt::DevModeAlgoManager;

    DevModeAlgoManager algo_manager = DevModeAlgoManager(algo_filenames);
    algo_manager.initialize_client_manager(users);
    for (auto& [_, client] : users.get_clients()) {
        std::visit(
            [&](auto&& arg) {
                using t = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<t, manager::local_trader_t>) {
                    arg.set_capital(1000000); // NOLINT(*)
                    arg.set_start_delay(has_delay);
                }
            },
            client
        );
    }
    size_t num_users = spawning::spawn_all_clients(users);
    rabbitmq::RabbitMQClientManager::wait_for_clients(users, num_users);
    rabbitmq::RabbitMQClientManager::send_start_time(users, CLIENT_WAIT_SECS);
}
} // namespace testing_utils
} // namespace nutc
