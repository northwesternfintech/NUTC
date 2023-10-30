#pragma once

#include "logging.hpp"
#include "spawning/spawning.hpp"

#include <crow/app.h>

#include <thread>

namespace nutc {
namespace crow {

std::thread get_server_thread();

}
} // namespace nutc
