#include "dev_mode.hpp"

#include <cassert>

#include <fstream>
#include <iostream>
#include <sstream>

namespace nutc {
namespace dev_mode {
bool
file_exists(const std::string& filename) noexcept
{
    std::ifstream file(filename);
    return file.good();
}

std::string
read_file_content(const std::string& filename)
{
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string
get_algo_from_file(const std::string& algo_id)
{
    std::string filename = fmt::format("./{}.py", algo_id);
    assert(file_exists(filename));
    return read_file_content(filename);
}
} // namespace dev_mode
} // namespace nutc
