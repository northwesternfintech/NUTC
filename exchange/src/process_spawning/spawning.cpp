#include "process_spawning/spawning.hpp"

#include "config.h"
#include "logging.hpp"
#include "curl/curl.hpp"
#include "utils/local_algos/dev_mode.hpp"

namespace nutc {
namespace client {

int
initialize(manager::ClientManager& users, Mode mode)
{
    if (mode == Mode::DEV) {
        dev_mode::initialize_client_manager(users, DEBUG_NUM_USERS);
        spawn_all_clients(users, mode);
        return DEBUG_NUM_USERS;
    }
    else if (mode == Mode::SANDBOX) {
        // dev_mode::initialize_client_manager
    //TODO
        return DEBUG_NUM_USERS;
    }
    else {
        // Get users from firebase
        glz::json_t::object_t firebase_users = nutc::client::get_all_users();
        users.initialize_from_firebase(firebase_users);

        // Spawn clients
        const int num_clients = nutc::client::spawn_all_clients(users, mode);

        if (num_clients == 0) {
            log_c(client_spawning, "Spawned 0 clients");
            exit(1);
        };
        return num_clients;
    }
}

glz::json_t::object_t
get_all_users()
{
    std::string endpoint = std::string(FIREBASE_URL) + std::string("users.json");
    glz::json_t res = curl::request_to_json("GET", endpoint);
    return res.get<glz::json_t::object_t>();
}

int
spawn_all_clients(const nutc::manager::ClientManager& users, Mode mode)
{
    int clients = 0;
    for (const auto& client : users.get_clients(false)) {
        const std::string uid = client.uid;
        log_i(client_spawning, "Spawning client: {}", uid);
        std::string quote_uid = std::string(uid);
        std::replace(quote_uid.begin(), quote_uid.end(), '-', ' ');
        spawn_client(quote_uid, mode);
        clients++;
    };
    return clients;
}

void
spawn_client(const std::string& uid, Mode mode)
{
    pid_t pid = fork();
    if (pid == 0) {
        std::vector<std::string> args = {"NUTC-client", "--uid", uid};
        if (mode == Mode::DEV) {
            args.push_back("--dev");
        }

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
