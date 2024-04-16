#pragma once

#include "exchange/traders/trader_manager.hpp"
#include "shared/util.hpp"

namespace nutc {
namespace algos {

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
class AlgoInitializer {
    /** Initialize any filesystem operations, if necessary
     * For example, dev_mode spawns in n algorithms in algos/algo_n.py
     */
    virtual void initialize_files() = 0;
    virtual void initialize_trader_container(manager::TraderManager& manager) const = 0;

public:
    void
    initialize_algo_management(manager::TraderManager& trader_container)
    {
        initialize_files();
        initialize_trader_container(trader_container);
    }

    static std::unique_ptr<AlgoInitializer>
    get_algo_initializer(util::Mode mode, std::optional<util::algorithm> sandbox);

    virtual ~AlgoInitializer() = default;
};

} // namespace algos
} // namespace nutc
