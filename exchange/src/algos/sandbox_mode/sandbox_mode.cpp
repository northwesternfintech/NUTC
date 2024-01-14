#include "sandbox_mode.hpp"

#include "curl/curl.hpp"
#include "logging.hpp"
#include "utils/file_operations/file_operations.hpp"

#include <glaze/glaze.hpp>

namespace nutc {
namespace algo_mgmt {
void
SandboxAlgoManager::initialize_client_manager(manager::ClientManager& users)
{
    num_clients_ = 0;

    // check number of algos in algos directory
    for (const auto& entry : std::filesystem::directory_iterator(ALGO_DIR)) {
        std::string algo_id = entry.path().filename().string();
        algo_id = algo_id.substr(0, algo_id.find(".py"));
        log_i(sandbox, "Adding client: {}", algo_id);
        users.add_client(algo_id, algo_id, /*is_local_algo=*/true);
        num_clients_ += 1;
    }

    users.add_client(user_id_, algo_id_, false);
    num_clients_ += 1;
}

void
SandboxAlgoManager::initialize_files() const
{
    std::string dir = ALGO_DIR;
    if (!file_ops::create_directory(dir))
        log_e(dev_mode, "{}", "Failed to create directory.");

    if (file_ops::file_exists(dir + "/algos.zip"))
        return;

    curl::request_to_file(
        "GET", "http://fintech-nutc.s3.us-east-2.amazonaws.com/algos.zip", "algos.zip"
    );

    file_ops::unzip_file("algos.zip", "algos");
}

} // namespace algo_mgmt
} // namespace nutc