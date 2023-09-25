#include "dev_mode.hpp"

namespace nutc {
namespace dev_mode {
void
initialize_client_manager(manager::ClientManager& users, int num_users)
{
    for (int i = 0; i < num_users; i++) {
        std::string uid = "user-" + std::to_string(i);
        users.addClient(uid);
    }
}

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

void
create_algo_files()
{
    std::string dir_name = "./algos";
    std::string file1_name = dir_name + "/algo_1.py";
    std::string file2_name = dir_name + "/algo_2.py";
    std::string content = read_file_content("./template.py");

    struct stat st;
    if (stat(dir_name.c_str(), &st) != 0) {
        if (mkdir(dir_name.c_str(), 0777) != 0) {
            std::cerr << "Failed to create directory.\n";
            return;
        }
    }

    if (!file_exists(file1_name)) {
        std::ofstream file1(file1_name);
        if (file1) {
            file1 << content;
            file1.close();
        }
        else {
            std::cerr << "Failed to create " << file1_name << "\n";
        }
    }

    if (!file_exists(file2_name)) {
        std::ofstream file2(file2_name);
        if (file2) {
            file2 << content;
            file2.close();
        }
        else {
            std::cerr << "Failed to create " << file2_name << "\n";
        }
    }
}
} // namespace dev_mode
} // namespace nutc
