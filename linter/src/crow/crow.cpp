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
            pid_t pid = fork();

            spawning::spawn_client(uid, algo_id, pid);

            // After 130 seconds, check status - if still pending, set failure and stop
            int status = 0;
            std::this_thread::sleep_for(std::chrono::seconds(130));
            pid_t result = waitpid(pid, &status, WNOHANG);

            // Some flags
            bool push_failure = (WIFEXITED(status) && WEXITSTATUS(status) != 0);

            if (push_failure) {
                // Kill process
                kill(pid, SIGKILL);

                // Push failure to Firebase
                nutc::client::set_lint_failure(
                    uid, algo_id, "Linting not completed after 130 seconds.\n"
                ); // TODO: pass ss str back from the process to have better error msg
            }

            return crow::response();
        });
        app.port(8080).run();
    });
    return server_thread;
}

} // namespace crow
} // namespace nutc
