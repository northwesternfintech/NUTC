#pragma once

#include "exchange/algos/algo_manager.hpp"
#include "exchange/traders/trader_manager.hpp"

#include <filesystem>
#include <limits>
#include <stdexcept>
#include <vector>

namespace nutc {
namespace algos {

namespace fs = std::filesystem;

class DevModeAlgoInitializer : public AlgoInitializer {
    // Limit to 255
    const uint8_t num_algos_;

    // Create the files ourselves if not provided
    std::vector<fs::path> algo_filepaths_;

public:
    constexpr explicit DevModeAlgoInitializer(uint8_t num_algos) :
        num_algos_(num_algos), algo_filepaths_({})
    {}

    explicit DevModeAlgoInitializer(const std::vector<fs::path>& algo_paths) :
        num_algos_(static_cast<uint8_t>(algo_paths.size())), algo_filepaths_(algo_paths)
    {
        size_t max_size = std::numeric_limits<uint8_t>::max();
        if (algo_paths.size() > max_size) {
            throw std::runtime_error(
                "Attempted to provide more algo filepaths than maximum"
            );
        }
    }

    void initialize_trader_container(manager::TraderManager& traders) const final;

    void initialize_files() final;
};

} // namespace algos
} // namespace nutc
