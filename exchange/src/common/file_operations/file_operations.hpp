#pragma once

#include <string>

namespace nutc::common {

bool file_exists(const std::string& filename) noexcept;

std::string read_file_content(const std::string& filename);

[[nodiscard]] bool create_directory(const std::string& dir);

void print_file_contents(const std::string& filepath);

} // namespace nutc::common
