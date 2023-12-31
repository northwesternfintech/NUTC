#include "sandbox.hpp"

#include "curl/curl.hpp"
#include "file_management.hpp"
#include "logging.hpp"

#include <glaze/glaze.hpp>

namespace nutc {
namespace sandbox {
size_t
initialize_client_manager(manager::ClientManager& users)
{
    // check number of algos in algos directory
    size_t num_users = 0;
    for (const auto& entry : std::filesystem::directory_iterator(ALGO_DIR)) {
        std::string algo_id = entry.path().filename().string();
        algo_id = algo_id.substr(0, algo_id.find(".py"));
        log_i(sandbox, "Adding client: {}", algo_id);
        users.add_client(algo_id, algo_id, /*is_local_algo=*/true);
        num_users += 1;
    }
    return num_users + 1;
}

void
create_sandbox_algo_files()
{
    std::string dir = ALGO_DIR;
    if (!file_mgmt::create_directory(dir))
        log_e(dev_mode, "{}", "Failed to create directory.");

    if (file_mgmt::file_exists(dir + "/algos.zip"))
        return;

    curl::request_to_file(
        "GET", "http://fintech-nutc.s3.us-east-2.amazonaws.com/algos.zip", "algos.zip"
    );

    file_mgmt::unzip_file("algos.zip", "algos");
}

} // namespace sandbox
} // namespace nutc
