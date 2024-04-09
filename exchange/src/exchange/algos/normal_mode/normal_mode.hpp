#pragma once

#include "exchange/algos/algo_manager.hpp"

namespace nutc {
namespace algo_mgmt {

class NormalModeAlgoManager : public AlgoManager {
    size_t num_clients_{};

public:
    void initialize_client_manager(manager::TraderManager& users) override;

    // No files to initialize
    void
    initialize_files() const override
    {}

private:
    static glz::json_t::object_t get_all_users();
};
} // namespace algo_mgmt
} // namespace nutc
