#pragma once

#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <optional>

namespace nutc {
namespace dev_mode {
bool file_exists(const std::string& filename);

std::string read_file_content(const std::string& filename);

std::optional<std::string> get_algo_from_file(const std::string& uid);
} // namespace dev_mode
} // namespace nutc
