#include "dev_mode.hpp"

#include "common/file_operations/file_operations.hpp"
#include "exchange/config/static/config.hpp"
#include "exchange/traders/trader_types/algo_trader.hpp"
#include "exchange/wrappers/creation/rmq_wrapper_init.hpp"

#include <stdexcept>

namespace nutc::exchange {

void
DevModeAlgoInitializer::initialize_trader_container(
    TraderContainer& traders, common::decimal_price start_capital
) const
{
    for (const common::LocalAlgorithm& algo : algorithms_) {
        traders.add_trader<AlgoTrader>(algo, start_capital);
    }

    int64_t start_time = get_start_time(WAIT_SECS);
    std::for_each(traders.begin(), traders.end(), [start_time](auto& trader) {
        send_start_time(trader, start_time);
    });
}

void
DevModeAlgoInitializer::initialize_files()
{
    for (size_t i = 0; i < NUM_ALGOS; i++) {
        auto relative_py_path = fmt::format("{}/algo_{}.py", ALGO_DIR, i);
        auto relative_cpp_path = fmt::format("{}/algo_{}.hpp", ALGO_DIR, i);
        algorithms_.emplace_back(common::AlgoLanguage::python, relative_py_path);
        algorithms_.emplace_back(common::AlgoLanguage::cpp, relative_cpp_path);
    }

    std::string py_content = common::read_file_content("template.py");
    std::string cpp_content = common::read_file_content("template.hpp");

    if (!common::create_directory(ALGO_DIR))
        throw std::runtime_error("Failed to create directory");

    for (const common::LocalAlgorithm& algo : algorithms_) {
        if (std::filesystem::exists(algo.get_path()))
            continue;

        std::ofstream file(algo.get_path());

        if (!file)
            throw std::runtime_error("Failed to create local algo");

        if (algo.get_language() == common::AlgoLanguage::python) {
            file << py_content;
        }
        else {
            file << cpp_content;
        }
        file.close();
    }
}

} // namespace nutc::exchange
