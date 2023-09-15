#pragma once

#include "firebase/firebase.hpp"

#include <glaze/glaze.hpp>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "client_manager/manager.hpp"

namespace nutc {
namespace client {

void spawn_client(const std::string& uid);
glz::json_t::object_t get_all_users();
int spawn_all_clients(const nutc::manager::ClientManager& users);

} // namespace client
} // namespace nutc
