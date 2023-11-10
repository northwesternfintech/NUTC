#include "file_management.hpp"

#include "logging.hpp"

#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <sstream>

namespace nutc {
namespace file_mgmt {

bool
create_directory(std::string dir_name)
{
    struct stat st;
    if (stat(dir_name.c_str(), &st) != 0) {
        if (mkdir(dir_name.c_str(), 0777) != 0) {
            log_e(dev_mode, "{}", "Failed to create directory.");
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
    std::ifstream file(filename);
    std::stringstream buffer;

    if (!file) {
        log_e(dev_mode, "File {} does not exist or could not be opened.", filename);
        return "";
    }

    buffer << file.rdbuf();
    return buffer.str();
}

} // namespace file_mgmt
} // namespace nutc
