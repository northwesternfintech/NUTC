#pragma once

#include <fmt/format.h>
#include <sys/stat.h>

#include <exception>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>

namespace nutc {
namespace dev_mode {
bool file_exists(const std::string& filename) noexcept;

std::string read_file_content(const std::string& filename);

std::string get_algo_from_file(const std::string& uid);
} // namespace dev_mode
} // namespace nutc
