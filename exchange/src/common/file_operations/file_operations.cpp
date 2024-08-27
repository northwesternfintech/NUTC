#include "file_operations.hpp"

#include <fmt/format.h>
#include <sys/stat.h>

#include <cassert>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

// Low level file operations. Decoupled from functionality of exchange
namespace nutc::common {

void
print_file_contents(const std::string& filepath)
{
    std::ifstream file(filepath);

    if (!file) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::cout << line << std::endl; // NOLINT
    }
}

bool
create_directory(const std::string& dir)
{
    std::filesystem::path dir_path{dir};
    if (!std::filesystem::exists(dir_path)) {
        if (!std::filesystem::create_directory(dir_path)) {
            return false;
        }
    }
    return true;
}

bool
file_exists(const std::string& filename) noexcept
{
    std::ifstream file(filename);
    return file.good();
}

std::string
read_file_content(const std::string& filename)
{
    if (!file_exists(filename)) {
        throw std::invalid_argument(fmt::format("Cannot find file {}", filename));
    }
    assert(file_exists(filename));
    std::ifstream file(filename);
    std::stringstream buffer;

    assert(file.good());

    buffer << file.rdbuf();
    return buffer.str();
}

} // namespace nutc::common
