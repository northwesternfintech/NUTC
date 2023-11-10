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
 * @brief Spawns a client process with the given uid
 * Forks and execve's a client process with the given uid
 * Spawns in the binary "NUTC-client", expecting it to be in the $PATH
 */
void spawn_client(const std::string& uid, Mode mode);

/**
 * @brief Fetches all users from firebase
 */
glz::json_t::object_t get_all_users();

/**
 * @brief Spawns all clients in the given ClientManager
 * @param users The ClientManager to spawn clients for
 * @returns the number of clients spawned
 */
int spawn_all_clients(const nutc::manager::ClientManager& users, Mode mode);

int initialize(manager::ClientManager& users, Mode mode);

} // namespace client
} // namespace nutc
