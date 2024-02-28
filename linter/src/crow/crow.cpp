#include "crow.hpp"

#include "common.hpp"
#include "config.h"
#include "firebase/fetching.hpp"

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
            crow::response res;

            // Set CORS headers
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header(
                "Access-Control-Allow-Methods", "GET, POST, PATCH, PUT, DELETE, OPTIONS"
            );
            res.add_header(
                "Access-Control-Allow-Headers", "Origin, Content-Type, Accept"
            );

            // Handle preflight request for OPTIONS method
            if (req.method == crow::HTTPMethod::OPTIONS) {
                res.code = 204; // No Content
                return res;
            }

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

            // Watchdog to check for crash/pending after
            // LINT_ABSOLUTE_TIMEOUT_SECONDS seconds (default 130s)
            std::thread check_pending_thread([algo_id, uid]() {
                std::this_thread::sleep_for(
                    std::chrono::seconds(LINT_ABSOLUTE_TIMEOUT_SECONDS)
                );

                // Check status from Firebase
                nutc::client::LintingResultOption linting_status =
                    nutc::client::get_algo_status(uid, algo_id);

                // If status is pending, force push a failure
                switch (linting_status) {
                    case nutc::client::LintingResultOption::PENDING:
                        {
                            // Push failure
                            std::string error_msg = fmt::format(
                                "Internal server error. Reach out to "
                                "nuft@u.northwestern.edu for support",
                                LINT_ABSOLUTE_TIMEOUT_SECONDS
                            );

                            nutc::client::set_lint_failure(uid, algo_id, error_msg);

                            log_e(
                                crow_watchdog,
                                "Algo id {} for uid {} still pending after {}s. FORCE "
                                "PUSHING "
                                "failure to Firebase.",
                                algo_id,
                                uid,
                                LINT_ABSOLUTE_TIMEOUT_SECONDS
                            );
                            break;
                        }
                    case nutc::client::LintingResultOption::UNKNOWN:
                        {
                            // can add a push to firebase here
                            log_e(
                                crow_watchdog,
                                "Algo id {} for uid {} unknown status after {}s.",
                                algo_id,
                                uid,
                                LINT_ABSOLUTE_TIMEOUT_SECONDS
                            );
                            nutc::client::set_lint_failure(
                                uid,
                                algo_id,
                                "Unknown status. Reach out to #nuft-support."
                            );
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
            });
            check_pending_thread.detach();

            spawning::spawn_client(uid, algo_id);

            for (int i = 0; i <= LINT_ABSOLUTE_TIMEOUT_SECONDS; i++) {
                if (nutc::client::get_algo_status(uid, algo_id)
                    != nutc::client::LintingResultOption::PENDING) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            res.body = "OK";
            res.code = 200;

            return res;
        });
        app.port(8080).run();
    });
    return server_thread;
}

} // namespace crow
} // namespace nutc
