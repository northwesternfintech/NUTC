#include "process.hpp"

#include "config.h"
#include "exchange/logging.hpp"
#include "exchange/process_spawning/spawning.hpp"
#include "exchange/rabbitmq/trader_manager/rmq_wrapper_init.hpp"
#include "test_utils/helpers/test_mode.hpp"

#include <future>
#include <ranges>

namespace nutc {
namespace test_utils {

bool
initialize_testing_clients(
    nutc::traders::TraderContainer& users,
    const std::vector<std::string>& algo_filenames, bool has_delay
)
{
    auto init_clients = [&]() {
        using algos::DevModeAlgoInitializer;

        std::vector<std::filesystem::path> algo_filepaths{};
        std::ranges::copy(algo_filenames, std::back_inserter(algo_filepaths));

        TestModeAlgoInitializer algo_manager{algo_filepaths};
        algo_manager.initialize_trader_container(users);
        std::ranges::for_each(
            users.get_traders(),
            [&has_delay](const auto& trader_pair) {
                const auto& trader = trader_pair.second;
                if (trader->get_type() == traders::TraderType::local) {
                    trader->set_start_delay(has_delay);
                }
            }
        );
        spawning::spawn_all_clients(users);
        rabbitmq::RabbitMQWrapperInitializer::wait_for_clients(users);
        rabbitmq::RabbitMQWrapperInitializer::send_start_time(
            users, TEST_CLIENT_WAIT_SECS
        );
        logging::init(quill::LogLevel::Info);
    };

    // Make sure clients are initialized within 100ms
    // This is just for testing utils, so it's okay

    auto future = std::async(std::launch::async, init_clients);
    return (
        future.wait_for(std::chrono::milliseconds(100)) != std::future_status::timeout
    );
}
} // namespace test_utils
} // namespace nutc
