#pragma once

#include "exchange/algos/algo_manager.hpp"

namespace nutc {
namespace algo_mgmt {

class NormalModeAlgoManager : public AlgoManager {
    size_t num_clients_{};

public:
    void initialize_client_manager(manager::ClientManager& users) override;

    // No files to initialize
    void
    initialize_files() const override
    {}

    // WARNING!! not initialized until initialize_client_manager is run
    size_t
    get_num_clients() const override
    {
        return num_clients_;
    }

private:
    static glz::json_t::object_t get_all_users();
};
} // namespace algo_mgmt
} // namespace nutc
