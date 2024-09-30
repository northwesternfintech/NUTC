#include "crow.hpp"

#include "fetching/fetching.hpp"
#include "logging.hpp"
#include "spawning/spawning.hpp"

namespace nutc {
namespace crow {

std::thread
get_server_thread()
{
    std::thread server_thread([]() {
        spawning::LintProcessManager spawner_manager;
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
            if (!req.url_params.get("language")) {
                log_e(main, "No language provided");
                return crow::response(400);
            }
            std::string uid = req.url_params.get("uid");
            std::string algo_id = req.url_params.get("algo_id");
            std::string language = req.url_params.get("language");

            spawning::AlgoLanguage algo_language;
            if (language == "python") {
                algo_language = spawning::AlgoLanguage::Python;
            }
            else if (language == "cpp") {
                algo_language = spawning::AlgoLanguage::Cpp;
            }
            else {
                log_e(main, "Invalid language provided: {}", language);
                return crow::response(400);
            }

            auto algo_code = nutc::client::get_algo(algo_id);
            if (!algo_code.has_value()) {
                nutc::client::set_lint_result(
                    uid,
                    algo_id,
                    false,
                    fmt::format(
                        "[linter] FAILURE - could not find algo {} for id {}\n",
                        algo_id,
                        uid
                    )
                );
                crow::json::wvalue response = crow::json::wvalue({
                    {"linting_status",
                     static_cast<int>(client::LintingResultOption::UNKNOWN)}
                });

                res.body = response.dump();
                res.code = 200;

                return res;
            }

            auto lint_res =
                spawner_manager.spawn_client(algo_code.value(), algo_language);

            nutc::client::set_lint_result(
                uid, algo_id, lint_res.success, lint_res.message
            );

            client::LintingResultOption algo_status_code =
                lint_res.success ? client::LintingResultOption::SUCCESS
                                 : client::LintingResultOption::FAILURE;
            crow::json::wvalue response({
                {"linting_status", static_cast<int>(algo_status_code)}
            });

            res.body = response.dump();
            res.code = 200;

            return res;
        });
        app.port(18081).run();
    });
    return server_thread;
}

} // namespace crow
} // namespace nutc
