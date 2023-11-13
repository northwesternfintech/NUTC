#include "crow.hpp"

#include "common.hpp"
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

            // Watchdog to check for crash/pending after 130s
            std::thread check_pending_thread([&algo_id = algo_id, &uid = uid]() {
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
                                "unknown runtime error: your code is syntactically "
                                "correct but timed out after {} seconds while linting",
                                LINT_ABSOLUTE_TIMEOUT_SECONDS
                            );

                            nutc::client::set_lint_result(uid, algo_id, false);
                            nutc::client::set_lint_failure(uid, algo_id, error_msg);

                            log_e(
                                main,
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
                                main,
                                "Algo id {} for uid {} unknown status after {}s.",
                                algo_id,
                                uid,
                                LINT_ABSOLUTE_TIMEOUT_SECONDS
                            );
                            break;
                        }
                    default:
                        {
                            log_i(
                                main,
                                "Algo id {} for uid {} looks ok {}s.",
                                algo_id,
                                uid,
                                LINT_ABSOLUTE_TIMEOUT_SECONDS
                            );
                            break;
                        }
                }
            });
            check_pending_thread.detach();

            spawning::spawn_client(uid, algo_id);

            return crow::response();
        });
        app.port(8080).run();
    });
    return server_thread;
}

} // namespace crow
} // namespace nutc
