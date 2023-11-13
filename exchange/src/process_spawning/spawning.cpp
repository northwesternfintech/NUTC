#include "process_spawning/spawning.hpp"

#include "config.h"
#include "curl/curl.hpp"
#include "local_algos/dev_mode.hpp"
#include "local_algos/sandbox.hpp"
#include "logging.hpp"

namespace nutc {
namespace client {

int
initialize(manager::ClientManager& users, Mode mode)
{
    int num_users;
    switch (mode) {
        case Mode::DEV:
            dev_mode::initialize_client_manager(users, DEBUG_NUM_USERS);
            spawn_all_clients(users);
            return DEBUG_NUM_USERS;
        case Mode::SANDBOX:
            num_users = sandbox::initialize_client_manager(users);
            spawn_all_clients(users);
            return num_users;
        default:
            // Get users from firebase
            glz::json_t::object_t firebase_users = nutc::client::get_all_users();
            users.initialize_from_firebase(firebase_users);

            // Spawn clients
            const int num_clients = nutc::client::spawn_all_clients(users);

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

std::string
quote_id(const std::string& id)
{
    std::string quote_id = std::string(id);
    std::replace(quote_id.begin(), quote_id.end(), '-', ' ');
    return quote_id;
}

int
spawn_all_clients(const nutc::manager::ClientManager& users)
{
    int clients = 0;
    for (const auto& client : users.get_clients(false)) {
        log_i(client_spawning, "Spawning client: {}", client.uid);
        std::string quoted_user_id = quote_id(client.uid);
        std::string quoted_algo_id = quote_id(client.algo_id);

        spawn_client(quoted_user_id, quoted_algo_id, client.is_local_algo);
        clients++;
    };
    return clients;
}

void
spawn_client(const std::string& uid, const std::string& algo_id, bool is_local_algo)
{
    pid_t pid = fork();
    if (pid == 0) {
        std::vector<std::string> args = {
            "NUTC-client", "--uid", uid, "--algo_id", algo_id
        };
        if (is_local_algo) {
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
