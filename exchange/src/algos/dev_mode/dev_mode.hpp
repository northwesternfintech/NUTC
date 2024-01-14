#pragma once

#include "algos/algo_manager.hpp"
#include "client_manager/client_manager.hpp"

#include <vector>

namespace nutc {
namespace algo_mgmt {

class DevModeAlgoManager : public AlgoManager {
    size_t num_clients_;
    std::optional<std::vector<std::string>> algo_filenames_;

public:
    explicit DevModeAlgoManager(size_t num_clients) : num_clients_(num_clients) {}

    explicit DevModeAlgoManager(
        size_t num_clients, const std::vector<std::string>& filenames
    ) :
        num_clients_(num_clients),
        algo_filenames_(filenames)
    {}

    void initialize_client_manager(manager::ClientManager& users) override;

    void initialize_files() const override;

    size_t
    get_num_clients() const override
    {
        return num_clients_;
    }
};

} // namespace algo_mgmt
} // namespace nutc
