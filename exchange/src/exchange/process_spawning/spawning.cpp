#include "spawning.hpp"

#include "exchange/logging.hpp"
#include "exchange/utils/file_operations/file_operations.hpp"

#include <cstdlib>

namespace nutc {
namespace client {

std::string
quote_id(std::string user_id)
{
    std::replace(user_id.begin(), user_id.end(), '-', ' ');
    return user_id;
}

size_t
spawn_all_clients(nutc::manager::ClientManager& users, SpawnMode mode)
{
    const char* wrapper_binary_location = std::getenv("NUTC_WRAPPER_BINARY_PATH");
    if (wrapper_binary_location == nullptr) [[unlikely]] {
        log_e(
            client_spawning,
            "Failed to get NUTC_WRAPPER_BINARY_PATH from environment variable"
        );
        exit(1);
    }
    const std::string wrapper_binary_path(wrapper_binary_location);

    size_t num_clients = 0;
    auto spawn_one_client = [&](const std::pair<std::string, manager::client_t>& pair) {
        const auto& [id, client] = pair;
        const std::string& algo_id = client.algo_id;

        if (client.active)
            return;

        log_i(client_spawning, "Spawning client: {}", id);

        pid_t pid = spawn_client(
            quote_id(id), quote_id(algo_id), client.algo_location, mode,
            wrapper_binary_path
        );
        users.set_client_pid(id, pid);
        num_clients++;
    };

    const auto& clients = users.get_clients();

    std::for_each(clients.begin(), clients.end(), spawn_one_client);

    return num_clients;
}

pid_t
spawn_client(
    const std::string& user_id, const std::string& algo_id,
    manager::ClientLocation algo_location, SpawnMode spawn_mode,
    const std::string& binary_path
)
{
    using manager::ClientLocation;

    if (algo_location == ClientLocation::LOCAL) {
        std::string filepath = algo_id + ".py";
        assert(file_ops::file_exists(filepath));
    }
    pid_t pid = fork();
    if (pid == 0) {
        std::vector<std::string> args = {
            binary_path, "--uid", user_id, "--algo_id", algo_id
        };
        if (algo_location == ClientLocation::LOCAL) {
            args.emplace_back("--dev");
        }
        if (spawn_mode == SpawnMode::TESTING) {
            args.emplace_back("--no-start-delay");
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
