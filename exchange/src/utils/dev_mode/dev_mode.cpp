#include "utils/dev_mode/dev_mode.hpp"

namespace nutc {
namespace dev_mode {
void
initialize_client_manager(manager::ClientManager& users, int num_users)
{
    for (int i = 0; i < num_users; i++) {
        std::string uid = "algo_" + std::to_string(i);
        users.add_client(uid);
    }
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

void
create_algo_files(int num_users) noexcept
{
    std::string dir_name = "./algos";
    std::string content = read_file_content("./template.py");
    struct stat st;

    if (stat(dir_name.c_str(), &st) != 0) {
        if (mkdir(dir_name.c_str(), 0777) != 0) {
            log_e(dev_mode, "{}", "Failed to create directory.");
            return;
        }
    }

    for (int i = 0; i < num_users; i++) {
        std::string file_name = dir_name + "/algo_" + std::to_string(i) + ".py";

        if (file_exists(file_name)) {
            continue;
        }

        log_i(dev_mode, "Creating default algo {}", file_name);
        std::ofstream file1(file_name);

        if (!file1) {
            log_e(dev_mode, "{}", "Failed to create file.");
            continue;
        }

        file1 << content;
        file1.close();
    }
}
} // namespace dev_mode
} // namespace nutc
