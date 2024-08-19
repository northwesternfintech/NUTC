#include "dev_mode.hpp"

#include "exchange/config/static/config.hpp"
#include "exchange/traders/trader_types/algo_trader.hpp"
#include "exchange/wrappers/creation/rmq_wrapper_init.hpp"
#include "shared/file_operations/file_operations.hpp"

#include <iostream>
#include <stdexcept>

namespace nutc {
namespace algos {

void
DevModeAlgoInitializer::initialize_trader_container(
    traders::TraderContainer& traders, double start_capital
) const
{
    for (const fs::path& filepath : algo_filepaths_)
        traders.add_trader<traders::AlgoTrader>(filepath, start_capital);

    int64_t start_time = rabbitmq::get_start_time(WAIT_SECS);
    std::for_each(traders.begin(), traders.end(), [start_time](auto& trader) {
        rabbitmq::send_start_time(trader, start_time);
    });
}

void
DevModeAlgoInitializer::initialize_files()
{
    if (!algo_filepaths_.empty())
        return;

    for (size_t i = 0; i < NUM_ALGOS; i++) {
        auto relative_path = fmt::format("{}/algo_{}.py", ALGO_DIR, i);
        algo_filepaths_.emplace_back(relative_path);
    }

    std::string content = file_ops::read_file_content("template.py");

    if (!file_ops::create_directory(ALGO_DIR))
        throw std::runtime_error("Failed to create directory");

    for (const fs::path& path : algo_filepaths_) {
        if (fs::exists(path))
            continue;

        std::ofstream file(path);

        if (!file)
            throw std::runtime_error("Failed to create local algo");

        file << content;
        file.close();
    }
}

} // namespace algos
} // namespace nutc
