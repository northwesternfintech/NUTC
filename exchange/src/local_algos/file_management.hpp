#pragma once

#include <string>

namespace nutc {
namespace file_mgmt {

void unzip_file(const std::string& src, const std::string& dest);

bool file_exists(const std::string& filename) noexcept;

std::string read_file_content(const std::string& filename);

[[nodiscard]] bool create_directory(const std::string& dir);

} // namespace file_mgmt
} // namespace nutc
