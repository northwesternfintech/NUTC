#include "dev_mode.hpp"

#include "config.h"
#include "file_management.hpp"
#include "logging.hpp"

namespace nutc {
namespace dev_mode {

void
initialize_client_manager(manager::ClientManager& users, int num_users)
{
    for (int i = 0; i < num_users; i++) {
        std::string uid = "algo_" + std::to_string(i);
        users.add_client(uid, true);
    }
}

void
create_mt_algo_files(int num_users) noexcept
{
    std::string content = file_mgmt::read_file_content("./template.py");
    std::string dir_name = std::string(ALGO_DIR);
    if (!file_mgmt::create_directory(dir_name))
        log_e(dev_mode, "{}", "Failed to create directory.");

    for (int i = 0; i < num_users; i++) {
        std::string file_name = dir_name + "/algo_" + std::to_string(i) + ".py";

        if (file_mgmt::file_exists(file_name)) {
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