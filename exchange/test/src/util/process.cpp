#include "process.hpp"

#include "common/types/decimal.hpp"
#include "exchange/algos/dev_mode/dev_mode.hpp"
#include "exchange/logging.hpp"

namespace nutc::test {
exchange::GenericTrader&
start_wrappers(
    nutc::exchange::TraderContainer& users, const common::LocalAlgorithm& algo,
    common::decimal_price starting_capital, size_t start_delay
)
{
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

    logging::init(quill::LogLevel::Info);

    DevModeAlgoInitializer algo_manager{start_delay, algos};
    algo_manager.initialize_trader_container(users, starting_capital);

    return {users.begin(), users.end()};
}
} // namespace nutc::test
