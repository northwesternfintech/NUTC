#pragma once

#include "logging.hpp"

#include <unistd.h>

#include <string>

namespace nutc {
namespace spawning {

void spawn_client(const std::string& uid, std::string& algoid, pid_t& pid);

}
} // namespace nutc
