#include "dev_mode.hpp"

#include "exchange/config.h"
#include "exchange/logging.hpp"
#include "shared/file_operations/file_operations.hpp"

#include <stdexcept>

namespace nutc {
namespace algo_mgmt {

void
DevModeAlgoManager::initialize_client_manager(manager::ClientManager& users)
{
    auto handle_algos_provided_filenames = [&]() {
        for (const std::string& filepath : algo_filenames_.value()) {
            users.add_local_trader(filepath, STARTING_CAPITAL);
        }
    };

    auto handle_algos_default_filenames = [&]() {
        for (size_t i = 0; i < num_clients_; i++) {
            std::string algo_id = std::string(ALGO_DIR) + "/algo_" + std::to_string(i);
            users.add_local_trader(algo_id, STARTING_CAPITAL);
        }
    };

    if (algo_filenames_.has_value())
        handle_algos_provided_filenames();
    else
        handle_algos_default_filenames();
}

void
DevModeAlgoManager::initialize_files() const
{
    if (algo_filenames_.has_value())
        return;

    std::string content = file_ops::read_file_content("template.py");
    std::string dir_name = std::string(ALGO_DIR);
    if (!file_ops::create_directory(dir_name)) {
        throw std::runtime_error("Failed to create directory");
    }

    for (size_t i = 0; i < num_clients_; i++) {
        std::string file_name = dir_name + "/algo_" + std::to_string(i) + ".py";

        if (file_ops::file_exists(file_name))
            continue;

        log_i(dev_mode, "Creating default algo {}", file_name);
        std::ofstream file1(file_name);

        if (!file1) {
            throw std::runtime_error("Failed to create file.");
        }

        file1 << content;
        file1.close();
    }
}

} // namespace algo_mgmt
} // namespace nutc
