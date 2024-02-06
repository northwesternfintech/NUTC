#pragma once

#include "exchange/algos/algo_manager.hpp"
#include "exchange/traders/trader_manager.hpp"

namespace nutc {
namespace algo_mgmt {

class SandboxAlgoManager : public AlgoManager {
    std::string user_id_;
    std::string algo_id_;
    size_t num_clients_{};

public:
    SandboxAlgoManager(std::string& user_id, std::string& algo_id) :
        user_id_(user_id), algo_id_(algo_id)
    {}

    void initialize_client_manager(manager::ClientManager& users) override;

    void initialize_files() const override;

    [[nodiscard]] size_t
    get_num_clients() const override
    {
        return num_clients_;
    }
};

} // namespace algo_mgmt
} // namespace nutc
