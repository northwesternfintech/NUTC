#include "process.hpp"

#include "exchange/algos/dev_mode/dev_mode.hpp"
#include "exchange/logging.hpp"
#include "exchange/wrappers/creation/rmq_wrapper_init.hpp"

namespace nutc {
namespace test {
std::shared_ptr<traders::GenericTrader>
start_wrappers(
    nutc::traders::TraderContainer& users, const std::string& filename,
    double starting_capital, size_t start_delay
)
{
    auto ret =
        start_wrappers(users, std::vector{filename}, starting_capital, start_delay);
    if (ret.size() != 1)
        throw std::runtime_error("Unexpected num wrappers");
    return ret[0];
}

std::pmr::vector<std::shared_ptr<traders::GenericTrader>>
start_wrappers(
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
} // namespace test
} // namespace nutc
