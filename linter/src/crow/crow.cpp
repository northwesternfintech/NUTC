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

            if (!req.url_params.get("algo_url")) {
                log_e(main, "No algo_id provided");
                return crow::response(400);
            }
            if (!req.url_params.get("language")) {
                log_e(main, "No language provided");
                return crow::response(400);
            }
            std::string algo_url = req.url_params.get("algo_url");
            std::string language = req.url_params.get("language");

            spawning::AlgoLanguage algo_language;
            if (language == "Python") {
                algo_language = spawning::AlgoLanguage::Python;
            }
            else if (language == "Cpp") {
                algo_language = spawning::AlgoLanguage::Cpp;
            }
            else {
                log_e(main, "Invalid language provided: {}", language);
                return crow::response(400);
            }

            auto algo_code = client::storage_request(algo_url);
            if (!algo_code.has_value()) {
                crow::json::wvalue response = crow::json::wvalue({
                    {"lint_success", false                },
                    {"message", "Algo file not found"}
                });

                res.body = response.dump();
                res.code = 200;

                return res;
            }

            auto lint_res =
                spawner_manager.spawn_client(algo_code.value(), algo_language);
            crow::json::wvalue response({
                {"lint_success", lint_res.success                                 },
                {"message", client::replaceDisallowedValues(lint_res.message)}
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
