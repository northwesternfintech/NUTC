#pragma once

#include "fetching.hpp"

#include <glaze/glaze.hpp>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace nutc {
namespace client {

void spawn_client(const std::string& uid);

// Should fetch all users from firebase and spawn the ones that have registered
// Client is expected to handle specific cases (multiple submitted algos, hasn't
// submitted algo, etc) So client should verify each uid is valid
void spawn_all_clients();

} // namespace client
} // namespace nutc
