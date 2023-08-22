#include "spawning.hpp"

#include "config.h"
#include "logging.hpp"

namespace nutc {
namespace client {
void
spawn_all_clients()
{
    std::string endpoint = std::string(FIREBASE_URL) + std::string("users.json");
    glz::json_t res = nutc::client::firebase_request("GET", endpoint);
    glz::json_t::object_t users = res.get<glz::json_t::object_t>();

    log_i(firebase_fetching, "Starting exchange with {} users", users.size());
    for (auto& [uid, user] : users) {
        log_i(firebase_fetching, "Spawning client: {}", uid);
        spawn_client(uid);
    };
}

void
spawn_client(const std::string& uid)
{
    pid_t pid = fork();
    if (pid == 0) {
        std::vector<std::string> args = {"NUTC-client", "-U", uid};
        std::vector<char*> c_args;
        for (auto& arg : args) {
            c_args.push_back(const_cast<char*>(arg.c_str()));
        }
        c_args.push_back(nullptr);
        execvp(c_args[0], &c_args[0]);

        log_e(firebase_fetching, "Failed to execute NUTC-client");

        exit(1);
    }
    else if (pid < 0) {
        log_e(firebase_fetching, "Failed to fork");
        exit(1);
    }
}
} // namespace client
} // namespace nutc
