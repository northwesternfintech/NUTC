#include "dev_mode.hpp"

namespace nutc {
namespace dev_mode {
bool
file_exists(const std::string& filename)
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

std::optional<std::string>
get_algo_from_file(const std::string& uid)
{
    std::string filename = "./algos/" + uid + ".py";
    if (!file_exists(filename)) {
        return std::nullopt;
    }
    return read_file_content(filename);
}
} // namespace dev_mode
} // namespace nutc
