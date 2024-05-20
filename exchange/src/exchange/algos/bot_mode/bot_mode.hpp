#pragma once

#include "exchange/algos/algo_manager.hpp"

namespace nutc {
namespace algos {

/**
 * @brief Does nothing on initialization
 */
class BotModeAlgoInitializer : public AlgoInitializer {
public:
    constexpr BotModeAlgoInitializer() = default;

    constexpr void
    initialize_trader_container(traders::TraderContainer&, double) const final
    {}

    constexpr void
    initialize_files() final
    {}
};

} // namespace algos
} // namespace nutc
