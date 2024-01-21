#pragma once

#include "exchange/client_manager/client_manager.hpp"

#include <glaze/glaze.hpp>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace nutc {

/** @brief Contains all functions related to spawning client processes */
namespace client {

enum class SpawnMode { NORMAL, TESTING };

/**
 * @brief Spawns a client process with the given id
 * Forks and execve's a client process with the given id
 * Spawns in the binary "NUTC-client", expecting it to be in the $PATH
 */
pid_t spawn_client(
    const std::string& user_id, const std::string& algo_id,
    manager::ClientLocation algo_location, SpawnMode spawn_mode
);

/**
 * @brief Spawns all clients in the given ClientManager
 * @param users The ClientManager to spawn clients for
 * @returns the number of clients spawned
 */
size_t spawn_all_clients(
    nutc::manager::ClientManager& users, SpawnMode mode = SpawnMode::NORMAL
);

} // namespace client
} // namespace nutc
