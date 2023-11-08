#include "crow.hpp"

#include "firebase/fetching.hpp"

#include <sys/wait.h>

namespace nutc {
namespace crow {

std::thread
get_server_thread()
{
    std::thread server_thread([]() {
        namespace crow = ::crow;
        ::crow::SimpleApp app;
        CROW_ROUTE(app, "/")
        ([&](const crow::request& req) {
            log_i(main, "Registered");

            if (!req.url_params.get("uid")) {
                log_e(main, "No uid provided");
                return crow::response(400);
            };
            if (!req.url_params.get("algo_id")) {
                log_e(main, "No algo_id provided");
                return crow::response(400);
            }
            std::string uid = req.url_params.get("uid");
            std::string algo_id = req.url_params.get("algo_id");
            pid_t pid = fork();

            spawning::spawn_client(uid, algo_id, pid);

            // After 130 seconds, check status - if still pending, set failure and stop
            std::this_thread::sleep_for(std::chrono::seconds(130));

            // Check status from Firebase
            std::optional<std::string> linting_status =
                nutc::client::get_algo_status(uid, algo_id);

            // If status is pending, force push a failure
            if (linting_status.has_value()) {
                if (linting_status == "pending") {
                    // Push failure
                    std::string error_msg =
                        "unknown runtime error: your code is syntactically correct but "
                        "crashed during runtime";

                    nutc::client::set_lint_result(uid, algo_id, false);
                    nutc::client::set_lint_failure(uid, algo_id, error_msg);
                }
            }

            return crow::response();
        });
        app.port(8080).run();
    });
    return server_thread;
}

} // namespace crow
} // namespace nutc
