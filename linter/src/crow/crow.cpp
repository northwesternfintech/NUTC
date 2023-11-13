#include "crow.hpp"

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
                std::this_thread::sleep_for(std::chrono::seconds(130));

                // Check status from Firebase
                nutc::client::LINTING_RESULT_OPTIONS linting_status =
                    nutc::client::get_algo_status(uid, algo_id);

                // If status is pending, force push a failure
                if (linting_status
                    == nutc::client::LINTING_RESULT_OPTIONS::LRO_PENDING) {
                    // TODO: include branches for LRO_UNKNOWN (?)
                    // Push failure
                    std::string error_msg =
                        "unknown runtime error: your code is syntactically correct but "
                        "timed out (130 seconds) while linting";

                    nutc::client::set_lint_result(uid, algo_id, false);
                    nutc::client::set_lint_failure(uid, algo_id, error_msg);
                }

                log_e(
                    main,
                    "Algoid {} for uid {} still pending after 130s. FORCE PUSHING "
                    "failure to "
                    "Firebase.",
                    algo_id,
                    uid
                );

                std::exit(1);
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
