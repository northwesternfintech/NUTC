#include "spawning.hpp"

#include "config.h"
#include "logging.hpp"

namespace nutc {
namespace client {

glz::json_t::object_t
get_all_users()
{
    std::string endpoint = std::string(FIREBASE_URL) + std::string("users.json");
    glz::json_t res = nutc::client::firebase_request("GET", endpoint);
    return res.get<glz::json_t::object_t>();
}

int
spawn_all_clients(const nutc::manager::ClientManager& users)
{
    int clients = 0;
    for (auto& [uid, user] : users.getClients(false)) {
        log_i(client_spawning, "Spawning client: {}", uid);
        std::string quote_uid = std::string(uid);
        std::replace(quote_uid.begin(), quote_uid.end(), '-', ' ');
        spawn_client(quote_uid);
        clients++;
    };
    return clients;
}

void
spawn_client(const std::string& uid)
{
    pid_t pid = fork();
    if (pid == 0) {
        std::vector<std::string> args = {"NUTC-client", "--uid", uid};

        std::vector<char*> c_args;
        for (auto& arg : args)
            c_args.push_back(arg.data());
        c_args.push_back(nullptr);

        execvp(c_args[0], c_args.data());

        log_e(client_spawning, "Failed to execute NUTC-client");

        exit(1);
    }
    else if (pid < 0) {
        log_e(client_spawning, "Failed to fork");
        exit(1);
    }
}
} // namespace client
} // namespace nutc
