#include "sandbox.hpp"

#include "file_management.hpp"
#include "logging.hpp"
#include "networking/curl/curl.hpp"

#include <glaze/glaze.hpp>

namespace nutc {
namespace sandbox {
void
create_sandbox_algo_files()
{
    std::string dir = ALGO_DIR;
    if (!file_mgmt::create_directory(dir))
        log_e(dev_mode, "{}", "Failed to create directory.");

    if (file_mgmt::file_exists(dir + "/algos.zip"))
        return;

    curl::request_to_file(
        "GET", "http://fintech-nutc.s3.us-east-2.amazonaws.com/algos.zip",
        "algos.zip"
    );

    file_mgmt::unzip_file("algos.zip", "algos");
}

} // namespace sandbox
} // namespace nutc
