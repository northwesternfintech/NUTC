#pragma once

#include "exchange/traders/trader_manager.hpp"

#include <glaze/glaze.hpp>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <filesystem>

namespace nutc {

/** @brief Contains all functions related to spawning client processes */
namespace spawning {
namespace fs = std::filesystem;

/**
 * @brief Spawns all clients in the given TraderManager
 * @param users The TraderManager to spawn clients for
 * @returns the number of clients spawned
 */
size_t spawn_all_clients(nutc::manager::TraderManager& users);
const fs::path& wrapper_binary_path();

} // namespace spawning
} // namespace nutc
