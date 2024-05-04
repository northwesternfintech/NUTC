#include "process.hpp"

#include "exchange/algos/dev_mode/dev_mode.hpp"
#include "exchange/logging.hpp"
#include "exchange/wrappers/creation/rmq_wrapper_init.hpp"

#include <future>

namespace nutc {
namespace test_utils {

[[nodiscard]] bool
initialize_testing_clients(
    nutc::traders::TraderContainer& users,
    const std::vector<std::string>& algo_filenames
)
{
    return initialize_testing_clients(users, algo_filenames, 0);
}

bool
initialize_testing_clients(
    nutc::traders::TraderContainer& users,
    const std::vector<std::string>& algo_filenames, size_t start_delay
)
{
    auto init_clients = [&]() {
        using algos::DevModeAlgoInitializer;

        std::vector<std::filesystem::path> algo_filepaths{};
        std::ranges::copy(algo_filenames, std::back_inserter(algo_filepaths));

        DevModeAlgoInitializer algo_manager{algo_filepaths};
        algo_manager.initialize_trader_container(users);
        logging::init(quill::LogLevel::Info);
        rabbitmq::WrapperInitializer::wait_for_clients(users);
        rabbitmq::WrapperInitializer::send_start_time(users, start_delay);
    };

    // Make sure clients are initialized within 100ms
    // This is just for testing utils, so it's okay

    auto launches_in_500ms = [&]() {
        namespace ch = std::chrono;
        auto future = std::async(std::launch::async, init_clients);
        return future.wait_until(ch::system_clock::now() + ch::milliseconds(500))
               != std::future_status::timeout;
    };
    return launches_in_500ms();
}
} // namespace test_utils
} // namespace nutc
