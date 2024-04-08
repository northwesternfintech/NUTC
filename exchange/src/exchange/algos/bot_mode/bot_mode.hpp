#pragma once

#include "exchange/algos/algo_manager.hpp"

namespace nutc {
namespace algo_mgmt {

/**
 * @brief Does nothing on initialization
 */
class BotModeAlgoManager : public AlgoManager {
public:
    constexpr void
    initialize_client_manager(manager::TraderManager&) override
    {}

    constexpr void
    initialize_files() const override
    {}
};

} // namespace algo_mgmt
} // namespace nutc
