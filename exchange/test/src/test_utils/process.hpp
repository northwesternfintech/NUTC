#pragma once

#include "client_manager/client_manager.hpp"

namespace nutc {
namespace testing_utils {

void kill_all_processes(const manager::ClientManager& users);

}
} // namespace nutc
