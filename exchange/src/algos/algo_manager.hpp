#pragma once

#include "client_manager/client_manager.hpp"

namespace nutc {
namespace algo_mgmt {

struct sandbox_params {
    std::string& user_id;
    std::string& algo_id;
};

/**
 * Serves as an abstract class to be implemented for dev_mode, sandbox_mode, and
 * normal_mode.
 * dev_mode spawns in local algorithms in exchange/algos and runs them
 *
 * sandbox_mode is meant to be run by docker, and spawns in local algorithms from remote
 * algos.zip plus one algo from firebase
 *
 * firebase_mode pulls all the algorithms from
 * firebase and runs them, and is the normal way of running NUTC in production
 */
class AlgoManager {
public:
    /** Initialize any filesystem operations, if necessary
     * For example, dev_mode spawns in n algorithms in algos/algo_n.py
     */
    virtual void initialize_files() const = 0;

    virtual void initialize_client_manager(manager::ClientManager& manager) = 0;

    virtual size_t get_num_clients() const = 0;

    virtual ~AlgoManager() = default;

    static size_t
    initialize_algo_management(
        nutc::algo_mgmt::AlgoManager& algo_manager,
        nutc::manager::ClientManager& client_manager
    )
    {
        algo_manager.initialize_files();
        algo_manager.initialize_client_manager(client_manager);
        return algo_manager.get_num_clients();
    }
};

} // namespace algo_mgmt
} // namespace nutc
