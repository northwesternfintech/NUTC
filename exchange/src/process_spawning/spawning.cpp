#include "process_spawning/spawning.hpp"

#include "config.h"
#include "curl/curl.hpp"
#include "local_algos/dev_mode.hpp"
#include "local_algos/file_management.hpp"
#include "local_algos/sandbox.hpp"
#include "logging.hpp"

namespace nutc {
namespace client {

size_t
initialize(manager::ClientManager& users, Mode mode, size_t num_local_algos)
{
    size_t num_users; // NOLINT(cppcoreguidelines-init-variables)
    switch (mode) {
        case Mode::DEV:
            dev_mode::initialize_client_manager(
                users, static_cast<int>(num_local_algos)
            );
            spawn_all_clients(users);
            return num_local_algos;
        case Mode::SANDBOX:
            num_users = sandbox::initialize_client_manager(users);
            spawn_all_clients(users);
            return num_users;
        default:
            // Get users from firebase
            glz::json_t::object_t firebase_users = nutc::client::get_all_users();
            users.initialize_from_firebase(firebase_users);

            // Spawn clients
            num_users = nutc::client::spawn_all_clients(users);

            if (num_users == 0) {
                log_c(client_spawning, "Spawned 0 clients");
                std::abort();
            };
            return num_users;
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
quote_id(std::string user_id)
{
    std::replace(user_id.begin(), user_id.end(), '-', ' ');
    return user_id;
}

size_t
spawn_all_clients(nutc::manager::ClientManager& users)
{
    size_t num_clients = 0;
    auto spawn_one_client =
        [&num_clients, &users](const std::pair<std::string, manager::client_t>& pair) {
            const auto& [id, client] = pair;
            const std::string& algo_id = client.algo_id;

            if (client.active)
                return;

            log_i(client_spawning, "Spawning client: {}", id);

            pid_t pid =
                spawn_client(quote_id(id), quote_id(algo_id), client.is_local_algo);
            users.set_client_pid(id, pid);
            num_clients++;
        };

    const auto& clients = users.get_clients();

    std::for_each(clients.begin(), clients.end(), spawn_one_client);

    return num_clients;
}

pid_t
spawn_client(const std::string& user_id, const std::string& algo_id, bool is_local_algo)
{
    if (is_local_algo) {
        std::string filepath = algo_id + ".py";
        assert(file_mgmt::file_exists(filepath));
    }
    pid_t pid = fork();
    if (pid == 0) {
        std::vector<std::string> args = {
            "NUTC-client", "--uid", user_id, "--algo_id", algo_id
        };
        if (is_local_algo) {
            args.emplace_back("--dev");
        }

        std::vector<char*> c_args;
        for (auto& arg : args) {
            c_args.emplace_back( // NOLINT(performance-inefficient-vector-operation)
                arg.data()
            );
        }
        c_args.push_back(nullptr);

        execvp(c_args[0], c_args.data());

        log_e(client_spawning, "Failed to execute NUTC-client");

        std::abort();
    }
    else if (pid < 0) {
        log_e(client_spawning, "Failed to fork");
        std::abort();
    }
    return pid;
}
} // namespace client
} // namespace nutc
