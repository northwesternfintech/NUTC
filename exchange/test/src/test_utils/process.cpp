#include "process.hpp"

#include "exchange/algos/dev_mode/dev_mode.hpp"
#include "exchange/config.h"
#include "exchange/logging.hpp"
#include "exchange/process_spawning/spawning.hpp"
#include "exchange/rabbitmq/client_manager/RabbitMQClientManager.hpp"

#include <future>

namespace nutc {
namespace testing_utils {

void
kill_all_processes(const manager::ClientManager& users)
{
    for (const auto& [id, trader] : users.get_traders()) {
        auto pid = trader->get_pid();
        if (pid != -1)
            kill(pid, SIGKILL);
    }
}

bool
initialize_testing_clients(
    nutc::manager::ClientManager& users, const std::vector<std::string>& algo_filenames,
    bool has_delay
)
{
    auto init_clients = [&]() {
        using algo_mgmt::DevModeAlgoManager;

        DevModeAlgoManager algo_manager = DevModeAlgoManager(algo_filenames);
        algo_manager.initialize_client_manager(users);
        for (auto& [_, trader] : users.get_traders()) {
            if (trader->get_type() == manager::LOCAL) {
                trader->set_capital(1000000);
                trader->set_start_delay(has_delay);
            }
        }
        size_t num_users = spawning::spawn_all_clients(users);
        rabbitmq::RabbitMQClientManager::wait_for_clients(users, num_users);
        rabbitmq::RabbitMQClientManager::send_start_time(users, CLIENT_WAIT_SECS);
        logging::init(quill::LogLevel::Info);
    };

    // Make sure clients are initialized within 100ms
    // This is just for testing utils, so it's okay

    auto future = std::async(std::launch::async, init_clients);
    return (
        future.wait_for(std::chrono::milliseconds(100)) != std::future_status::timeout
    );
}
} // namespace testing_utils
} // namespace nutc
