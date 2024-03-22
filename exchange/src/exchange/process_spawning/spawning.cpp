#include "spawning.hpp"

#include "exchange/logging.hpp"
#include "shared/file_operations/file_operations.hpp"

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
spawn_client(
    const std::shared_ptr<manager::GenericTrader>& trader,
    const std::string& binary_path
)
{
    if (trader->get_type() == manager::LOCAL) {
        const std::string filepath = trader->get_algo_id() + ".py";
        assert(file_ops::file_exists(filepath));
    }

    pid_t pid = fork();
    if (pid == 0) {
        std::vector<std::string> args = {
            binary_path, "--uid", quote_id(trader->get_id()), "--algo_id",
            quote_id(trader->get_algo_id())
        };

        if (trader->get_type() == manager::LOCAL) {
            args.emplace_back("--dev");
        }

        if (!trader->has_start_delay()) {
            args.emplace_back("--no-start-delay");
        }

        std::vector<char*> c_args;
        c_args.reserve(args.size() + 1);
        for (auto& arg : args) {
            c_args.push_back(arg.data());
        }
        c_args.push_back(nullptr);

        execvp(c_args[0], c_args.data());
        log_e(client_spawning, "Failed to execute NUTC wrapper");
        std::abort();
    }
    else if (pid < 0) { // Fork failed
        log_e(client_spawning, "Failed to fork");
        std::abort();
    }

    return pid;
}

size_t
spawn_all_clients(nutc::manager::TraderManager& users)
{
    const char* wrapper_binary_location = std::getenv("NUTC_WRAPPER_BINARY_PATH");
    assert(
        wrapper_binary_location != nullptr
        && "NUTC_WRAPPER_BINARY_PATH environment variable not set"
    );

    const std::string wrapper_binary_path(wrapper_binary_location);
    assert(
        file_ops::file_exists(wrapper_binary_path)
        && "NUTC_WRAPPER_BINARY_PATH does not exist"
    );

    size_t num_clients = 0;
    auto spawn_one_trader = [&](const std::shared_ptr<manager::GenericTrader>& trader) {
        if (trader->get_type() == manager::BOT)
            return;

        if (trader->is_active())
            return;

        const std::string& trader_id = trader->get_id();
        log_i(client_spawning, "Spawning client: {}", trader_id);

        std::string algo_id;
        trader->set_pid(spawn_client(trader, wrapper_binary_path));
        num_clients++;
    };

    for (const auto& [id, trader] : users.get_traders()) {
        spawn_one_trader(trader);
    }

    return num_clients;
}

} // namespace spawning
} // namespace nutc
