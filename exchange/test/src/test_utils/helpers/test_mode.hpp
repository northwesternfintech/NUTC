#pragma once

#include "exchange/algos/dev_mode/dev_mode.hpp"
#include "exchange/traders/trader_container.hpp"

#include <filesystem>
#include <limits>
#include <stdexcept>
#include <vector>

namespace nutc {
namespace test_utils {

namespace fs = std::filesystem;

// Mostly just uses DevMode algo initializer
class TestModeAlgoInitializer : public algos::DevModeAlgoInitializer {
public:
    constexpr explicit TestModeAlgoInitializer(uint8_t num_algos) :
        DevModeAlgoInitializer(num_algos)
    {}

    explicit TestModeAlgoInitializer(const std::vector<fs::path>& algo_paths) :
        DevModeAlgoInitializer(algo_paths)
    {}

    void initialize_trader_container(traders::TraderContainer& traders) const final;
};

} // namespace test_utils
} // namespace nutc
