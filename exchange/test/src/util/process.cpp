#include "process.hpp"

#include "common/logging/logging.hpp"
#include "common/types/decimal.hpp"
#include "exchange/algos/dev_mode/dev_mode.hpp"

#include <fmt/format.h>

namespace nutc::test {
namespace {
std::string
get_relative_path(common::AlgoLanguage language, const std::string& algo_name)
{
    if (language == common::AlgoLanguage::cpp) {
        return fmt::format("test_algos/cpp/{}.hpp", algo_name);
    }
    return fmt::format("test_algos/python/{}.py", algo_name);
}
} // namespace

exchange::GenericTrader&
start_wrappers(
    nutc::exchange::TraderContainer& users, common::AlgoLanguage language,
    const std::string& algo_name, common::decimal_price starting_capital,
    size_t start_delay
)
{
    common::LocalAlgorithm algo{language, get_relative_path(language, algo_name)};
    auto ret = start_wrappers(users, std::vector{algo}, starting_capital, start_delay);
    if (ret.size() != 1)
        throw std::runtime_error("Unexpected num wrappers");
    return ret[0];
}

std::vector<std::reference_wrapper<exchange::GenericTrader>>
start_wrappers(
    nutc::exchange::TraderContainer& users,
    const std::vector<common::LocalAlgorithm>& algos,
    common::decimal_price starting_capital, size_t start_delay
)
{
    using exchange::DevModeAlgoInitializer;

    common::logging_init("integration_tests.log", quill::LogLevel::Info);

    DevModeAlgoInitializer algo_manager{start_delay, algos};
    algo_manager.initialize_trader_container(users, starting_capital);

    return {users.begin(), users.end()};
}
} // namespace nutc::test
