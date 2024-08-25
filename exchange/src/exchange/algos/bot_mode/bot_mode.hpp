#pragma once

#include "exchange/algos/algo_manager.hpp"

namespace nutc::exchange {

/**
 * @brief Does nothing on initialization
 */
class BotModeAlgoInitializer : public AlgoInitializer {
public:
    constexpr BotModeAlgoInitializer() = default;

    constexpr void
    initialize_trader_container(TraderContainer&, shared::decimal_price) const final
    {}

    constexpr void
    initialize_files() final
    {}
};

} // namespace nutc::exchange
