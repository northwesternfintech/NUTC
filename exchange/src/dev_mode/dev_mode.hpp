#pragma once

#include "client_manager/client_manager.hpp"
#include "config.h"
#include "logging.hpp"

#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <sstream>

namespace nutc {
namespace dev_mode {
bool file_exists(const std::string& path) noexcept;
std::string read_file(const std::string& path);
void create_algo_files(int num_users) noexcept;
void initialize_client_manager(manager::ClientManager& users, int num_users);
} // namespace dev_mode
} // namespace nutc
