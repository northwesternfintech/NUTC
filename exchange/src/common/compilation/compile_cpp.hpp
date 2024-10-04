#pragma once

#include <boost/filesystem.hpp>
#include <fmt/format.h>

#include <filesystem>
#include <string>

namespace nutc::common {

// TODO: shouldnt return filepath as string
std::string compile_cpp(const std::filesystem::path& filepath);
} // namespace nutc::common
