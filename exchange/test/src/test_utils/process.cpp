#include "process.hpp"

#include "exchange/algos/dev_mode/dev_mode.hpp"
#include "exchange/logging.hpp"
#include "exchange/wrappers/creation/rmq_wrapper_init.hpp"

namespace nutc {
namespace test_utils {

std::vector<std::shared_ptr<traders::GenericTrader>>
initialize_testing_clients(
    nutc::traders::TraderContainer& users,
    const std::vector<std::string>& algo_filenames, double starting_capital,
    size_t start_delay
)
{
    using algos::DevModeAlgoInitializer;

    std::vector<std::filesystem::path> algo_filepaths{};
    std::ranges::copy(algo_filenames, std::back_inserter(algo_filepaths));

    DevModeAlgoInitializer algo_manager{algo_filepaths};
    algo_manager.initialize_trader_container(users, starting_capital);
    logging::init(quill::LogLevel::Info);
    rabbitmq::WrapperInitializer::send_start_time(users.get_traders(), start_delay);

    return users.get_traders();
}
} // namespace test_utils
} // namespace nutc
