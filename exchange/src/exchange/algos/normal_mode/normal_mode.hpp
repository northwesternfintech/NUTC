#pragma once

#include "exchange/algos/algo_manager.hpp"

namespace nutc {
namespace algos {

class NormalModeAlgoInitializer : public AlgoInitializer {
public:
    constexpr NormalModeAlgoInitializer() = default;

    void initialize_trader_container(
        traders::TraderContainer& traders, double start_capital
    ) const override;

    // No files to initialize
    constexpr void
    initialize_files() final
    {}

private:
    static glz::json_t::object_t get_remote_traders();
};
} // namespace algos
} // namespace nutc
