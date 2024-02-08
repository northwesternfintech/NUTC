#pragma once

#include "exchange/traders/trader_manager.hpp"

#include <glaze/glaze.hpp>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace nutc {

/** @brief Contains all functions related to spawning client processes */
namespace spawning {

/**
 * @brief Spawns all clients in the given ClientManager
 * @param users The ClientManager to spawn clients for
 * @returns the number of clients spawned
 */
size_t spawn_all_clients(nutc::manager::ClientManager& users);

} // namespace spawning
} // namespace nutc
