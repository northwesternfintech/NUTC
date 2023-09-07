#pragma once

#include "fetching.hpp"

#include <glaze/glaze.hpp>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace nutc {
namespace client {

void spawn_client(const std::string& uid);
glz::json_t::object_t get_all_users();
int spawn_all_clients(glz::json_t::object_t users);

} // namespace client
} // namespace nutc
