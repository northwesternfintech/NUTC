#include "dev_mode.hpp"

#include "common/file_operations/file_operations.hpp"
#include "common/util.hpp"
#include "exchange/config/static/config.hpp"
#include "exchange/traders/trader_types/algo_trader.hpp"
#include "exchange/wrappers/creation/rmq_wrapper_init.hpp"

#include <iostream>
#include <stdexcept>

namespace nutc::exchange {

void
DevModeAlgoInitializer::initialize_trader_container(
    TraderContainer& traders, common::decimal_price start_capital
) const
{
    for (const auto& [filepath, algo_type] : algo_filepaths_) {
        WrapperHandle handle(filepath, algo_type);
        traders.add_trader<AlgoTrader>(filepath, start_capital, std::move(handle));
    }

    int64_t start_time = get_start_time(WAIT_SECS);
    std::for_each(traders.begin(), traders.end(), [start_time](auto& trader) {
        send_start_time(trader, start_time);
    });
}

void
DevModeAlgoInitializer::initialize_files()
{
    if (!algo_filepaths_.empty())
        return;

    for (size_t i = 0; i < NUM_ALGOS; i++) {
        auto relative_path = fmt::format("{}/algo_{}.py", ALGO_DIR, i);
        algo_filepaths_.emplace_back(relative_path, common::AlgoType::python);
    }

    std::string content = common::read_file_content("template.py");

    if (!common::create_directory(ALGO_DIR))
        throw std::runtime_error("Failed to create directory");

    for (const auto& [path, _] : algo_filepaths_) {
        if (fs::exists(path))
            continue;

        std::ofstream file(path);

        if (!file)
            throw std::runtime_error("Failed to create local algo");

        file << content;
        file.close();
    }
}

} // namespace nutc::exchange
