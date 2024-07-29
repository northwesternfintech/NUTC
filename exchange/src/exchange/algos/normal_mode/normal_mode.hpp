#pragma once

#include "exchange/algos/algo_manager.hpp"

namespace nutc {
namespace algos {

class NormalModeAlgoInitializer : public AlgoInitializer {
public:
    const size_t WAIT_SECS;

    NormalModeAlgoInitializer(size_t wait_secs) : WAIT_SECS(wait_secs) {}

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
