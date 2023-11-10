#include "sandbox.hpp"

#include "file_management.hpp"
#include "logging.hpp"
#include <glaze/glaze.hpp>
#include "networking/firebase/firebase.hpp"

namespace nutc {
namespace sandbox {
void
create_sandbox_algo_files()
{
    if (!file_mgmt::create_directory(ALGO_DIR))
        log_e(dev_mode, "{}", "Failed to create directory.");

    if (file_mgmt::file_exists("algos.zip"))
        return;

    glz::json_t zip_file = firebase::firebase_request(
        "GET", "https://fintech-nutc.s3.us-east-2.amazonaws.com/algos.zip"
    );
    std::string buf;
    glz::write_json(zip_file, buf);
    log_e(dev_mode, "{}", buf);
}

} // namespace sandbox
} // namespace nutc
