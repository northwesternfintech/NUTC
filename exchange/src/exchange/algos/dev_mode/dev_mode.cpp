#include "dev_mode.hpp"

#include "exchange/config.h"
#include "exchange/traders/trader_types/local_trader.hpp"
#include "shared/config/config_loader.hpp"
#include "shared/file_operations/file_operations.hpp"

#include <stdexcept>

namespace nutc {
namespace algos {

void
DevModeAlgoInitializer::initialize_trader_container(manager::TraderManager& traders
) const
{
    int starting_cap = config::Config::get_instance().constants().STARTING_CAPITAL;

    for (const fs::path& filepath : algo_filepaths_) {
        traders.add_trader<manager::LocalTrader>(filepath.string(), starting_cap);
    }
}

void
DevModeAlgoInitializer::initialize_files()
{
    if (!algo_filepaths_.empty())
        return;

    for (uint8_t i = 0; i < num_algos_; i++) {
        fs::path path{std::string(ALGO_DIR) + "/algo_" + std::to_string(i) + ".py"};
        algo_filepaths_.push_back(path);
    }

    std::string content = file_ops::read_file_content("template.py");

    if (!file_ops::create_directory(ALGO_DIR)) {
        throw std::runtime_error("Failed to create directory");
    }

    for (const fs::path& path : algo_filepaths_) {
        if (fs::exists(path))
            continue;

        std::ofstream file1(path);

        if (!file1) {
            throw std::runtime_error("Failed to create local algo");
        }

        file1 << content;
        file1.close();
    }
}

} // namespace algos
} // namespace nutc
