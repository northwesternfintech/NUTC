#pragma once

#include "client_manager/client_manager.hpp"

#include <glaze/glaze.hpp>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace nutc {

/** @brief Contains all functions related to spawning client processes */
namespace client {

/**
 * @brief Spawns a client process with the given id
 * Forks and execve's a client process with the given id
 * Spawns in the binary "NUTC-client", expecting it to be in the $PATH
 */
pid_t
spawn_client(const std::string& id, const std::string& algo_id, bool is_local_algo);

/**
 * @brief Fetches all users from firebase
 */
glz::json_t::object_t get_all_users();

/**
 * @brief Spawns all clients in the given ClientManager
 * @param users The ClientManager to spawn clients for
 * @returns the number of clients spawned
 */
size_t spawn_all_clients(nutc::manager::ClientManager& users);

size_t initialize(
    manager::ClientManager& users, Mode mode, size_t num_local_algos = DEBUG_NUM_USERS
);

} // namespace client
} // namespace nutc
