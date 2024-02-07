#include "spawning.hpp"

#include "exchange/logging.hpp"
#include "exchange/traders/trader_types.hpp"
#include "exchange/utils/file_operations/file_operations.hpp"

#include <cstdlib>

namespace nutc {
namespace spawning {

std::string
quote_id(std::string user_id)
{
    std::replace(user_id.begin(), user_id.end(), '-', ' ');
    return user_id;
}

pid_t
spawn_client(auto& trader, const std::string& binary_path)
{
    using t = std::decay_t<decltype(trader)>;
    if constexpr (std::is_same_v<t, nutc::manager::local_trader_t>) {
        std::string filepath = trader.get_algo_path() + ".py";
        assert(file_ops::file_exists(filepath));
    }

    pid_t pid = fork();
    if (pid == 0) {
        std::vector<std::string> args;
        if constexpr (std::is_same_v<t, nutc::manager::local_trader_t>) {
            args = {binary_path,
                    "--uid",
                    trader.get_id(),
                    "--algo_id",
                    trader.get_algo_path(),
                    "--dev"};
        }
        else {
            args = {
                binary_path, "--uid", trader.get_id(), "--algo_id", trader.get_algo_id()
            };
        }

        if (!trader.has_start_delay())
            args.emplace_back("--no-start-delay");

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

size_t
spawn_all_clients(nutc::manager::ClientManager& users)
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
    auto spawn_one_client = [&](auto&& arg) {
        using t = std::decay_t<decltype(arg)>;

        if (arg.is_active())
            return;

        if constexpr (!std::is_same_v<t, nutc::manager::bot_trader_t>) {
            const std::string& trader_id = arg.get_id();
            log_i(client_spawning, "Spawning client: {}", trader_id);

            std::string algo_id;
            arg.set_pid(spawn_client(arg, wrapper_binary_path));
        }
        num_clients++;
    };

    for (auto& [id, client] : users.get_clients()) {
        std::visit(spawn_one_client, client);
    }

    return num_clients;
}

} // namespace spawning
} // namespace nutc
