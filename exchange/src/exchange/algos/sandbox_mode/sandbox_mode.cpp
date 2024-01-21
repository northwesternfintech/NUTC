#include "sandbox_mode.hpp"

#include "exchange/curl/curl.hpp"
#include "logging.hpp"
#include "exchange/utils/file_operations/file_operations.hpp"

#include <glaze/glaze.hpp>

namespace nutc {
namespace algo_mgmt {
void
SandboxAlgoManager::initialize_client_manager(manager::ClientManager& users)
{
    using manager::ClientLocation;

    num_clients_ = 0;

    // check number of algos in algos directory
    for (const auto& entry : std::filesystem::directory_iterator(ALGO_DIR)) {
        std::string algo_id = entry.path().filename().string();
        algo_id = algo_id.substr(0, algo_id.find(".py"));
        log_i(sandbox, "Adding client: {}", algo_id);
        users.add_client(algo_id, algo_id, ClientLocation::LOCAL);
        num_clients_ += 1;
    }

    users.add_client(user_id_, algo_id_, ClientLocation::REMOTE);
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
