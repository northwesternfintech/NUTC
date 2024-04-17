#include "process.hpp"

#include "config.h"
#include "exchange/algos/dev_mode/dev_mode.hpp"
#include "exchange/logging.hpp"
#include "exchange/process_spawning/spawning.hpp"
#include "exchange/rabbitmq/trader_manager/RabbitMQTraderManager.hpp"

#include <future>

namespace nutc {
namespace testing_utils {

void
kill_all_processes(const manager::TraderManager& users)
{
    for (const auto& [id, trader] : users.get_traders()) {
        auto pid = trader->get_pid();
        if (pid != -1)
            kill(pid, SIGKILL);
    }
}

bool
initialize_testing_clients(
    nutc::manager::TraderManager& users, const std::vector<std::string>& algo_filenames,
    bool has_delay
)
{
    auto init_clients = [&]() {
        using algos::DevModeAlgoInitializer;

        std::vector<std::filesystem::path> algo_filepaths;
        algo_filepaths.reserve(algo_filenames.size());
        for (const std::string& path : algo_filenames) {
            algo_filepaths.emplace_back(path);
        }

        DevModeAlgoInitializer algo_manager{algo_filepaths};
        algo_manager.initialize_trader_container(users);
        for (auto& [_, trader] : users.get_traders()) {
            if (trader->get_type() == manager::TraderType::local) {
                trader->set_start_delay(has_delay);
            }
        }
        spawning::spawn_all_clients(users);
        rabbitmq::RabbitMQTraderManager::wait_for_clients(users);
        rabbitmq::RabbitMQTraderManager::send_start_time(users, TEST_CLIENT_WAIT_SECS);
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
