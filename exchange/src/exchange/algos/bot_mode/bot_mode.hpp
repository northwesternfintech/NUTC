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

    [[nodiscard]] constexpr size_t
    get_num_clients() const override
    {
        return 0;
    }
};

} // namespace algo_mgmt
} // namespace nutc
