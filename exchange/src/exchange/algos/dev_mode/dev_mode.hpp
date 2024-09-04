#pragma once

#include "common/types/decimal.hpp"
#include "common/util.hpp"
#include "exchange/algos/algo_manager.hpp"
#include "exchange/traders/trader_container.hpp"

#include <filesystem>
#include <limits>
#include <stdexcept>
#include <vector>

namespace nutc::exchange {
using algo_type = common::AlgoType;

namespace fs = std::filesystem;

class DevModeAlgoInitializer : public AlgoInitializer {
protected:
    const size_t WAIT_SECS;

    // Limit to 255
    const uint8_t NUM_ALGOS;

    // Create the files ourselves if not provided
    std::vector<std::pair<fs::path, algo_type>> algo_filepaths_;

public:
    explicit DevModeAlgoInitializer(size_t wait_secs, uint8_t num_algos) :
        WAIT_SECS(wait_secs), NUM_ALGOS(num_algos), algo_filepaths_({})
    {}

    explicit DevModeAlgoInitializer(
        size_t wait_secs, const std::vector<std::pair<fs::path, algo_type>>& algo_paths
    ) :
        WAIT_SECS(wait_secs), NUM_ALGOS(static_cast<uint8_t>(algo_paths.size())),
        algo_filepaths_(algo_paths)
    {
        size_t max_size = std::numeric_limits<uint8_t>::max();
        if (algo_paths.size() > max_size) {
            throw std::runtime_error(
                "Attempted to provide more algo filepaths than maximum"
            );
        }
    }

    void initialize_trader_container(
        TraderContainer& traders, common::decimal_price start_capital
    ) const override;

    void initialize_files() final;
};

} // namespace nutc::exchange
