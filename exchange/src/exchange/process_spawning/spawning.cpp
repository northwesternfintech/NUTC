#include "spawning.hpp"

#include "exchange/logging.hpp"
#include "exchange/process_spawning/wrapper_handle.hpp"
#include "exchange/traders/trader_types/trader_interface.hpp"
#include "shared/file_operations/file_operations.hpp"

#include <boost/process.hpp>

#include <cstdlib>

namespace nutc {
namespace spawning {
namespace bp = boost::process;

namespace {
std::string
quote_id(std::string user_id)
{
    std::replace(user_id.begin(), user_id.end(), '-', ' ');
    return user_id;
}

struct algo_spawn_results {
    bp::child prox;
    bp::opstream pipe_to_child;
};
} // namespace

const fs::path&
wrapper_binary_path()
{
    static const char* wrapper_binary_location =
        std::getenv("NUTC_WRAPPER_BINARY_PATH"); // NOLINT
    if (wrapper_binary_location == nullptr) [[unlikely]] {
        throw std::runtime_error("NUTC_WRAPPER_BINARY_PATH environment variable not set"
        );
    }

    static const fs::path wrapper_binary_path{wrapper_binary_location};
    if (!fs::exists(wrapper_binary_path))
        throw std::runtime_error("File at NUTC_WRAPPER_BINARY_PATH does not exist");

    return wrapper_binary_path;
}

algo_spawn_results
spawn_algo_wrapper(const std::shared_ptr<traders::GenericTrader>& trader)
{
    static const fs::path& wrapper_binpath = wrapper_binary_path();

    std::vector<std::string> args{
        "--uid", quote_id(trader->get_id()), "--algo_id",
        quote_id(trader->get_algo_id())
    };

    if (trader->get_type() == traders::TraderType::local)
        args.emplace_back("--dev");

    if (!trader->has_start_delay())
        args.emplace_back("--no-start-delay");

    bp::opstream parent_to_child{};
    bp::child c{
        bp::exe(std::string{wrapper_binpath}), bp::args(args),
        bp::std_in<parent_to_child, bp::std_err> stderr, bp::std_out > stdout
    };

    return {std::move(c), std::move(parent_to_child)};
}

size_t
spawn_all_clients(nutc::traders::TraderContainer& users)
{
    size_t num_clients{};
    auto spawn_one_trader = [&](const auto& trader_pair) {
        const auto& [id, trader] = trader_pair;
        // Bots do not have algo wrappers
        if (trader->get_type() == traders::TraderType::bot)
            return;

        if (trader->is_active())
            return;

        auto spawn_results = spawn_algo_wrapper(trader);
        WrapperHandle handle{
            std::move(spawn_results.prox), std::move(spawn_results.pipe_to_child)
        };
        trader->set_wrapper_handle(std::move(handle));
        num_clients++;
    };

    std::ranges::for_each(users.get_traders(), spawn_one_trader);

    return num_clients;
}

} // namespace spawning
} // namespace nutc
