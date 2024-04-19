#include "argparse.hpp"

#include "shared/config/config.h"

namespace nutc {
namespace config {

std::tuple<mode, std::optional<algorithm>>
process_arguments(int argc, const char** argv)
{
    argparse::ArgumentParser program(
        "NUTC24", VERSION, argparse::default_arguments::help
    );

    program.add_argument("-D", "--dev")
        .help("Enable development features")
        .action([](const auto& /* unused */) {})
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

    program.add_argument("--bots-only")
        .help("No algos spawned in, just bots")
        .action([](const auto& /* unused */) {})
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

    program.add_argument("-S", "--sandbox").help("Provide a sandbox algo id").nargs(2);

    program.add_argument("-V", "--version")
        .help("prints version information and exits")
        .action([&](const auto& /* unused */) {
            fmt::println("NUTC24  v{}", VERSION);
            exit(0); // NOLINT(concurrency-*)
        })
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl; // NOLINT
        std::cerr << program;
        exit(1); // NOLINT(concurrency-*)
    }

    auto get_sandbox_algo = [&]() -> std::optional<algorithm> {
        if (!program.is_used("--sandbox")) {
            return std::nullopt;
        }
        auto sandbox = program.get<std::vector<std::string>>("--sandbox");
        if (sandbox.size() != 2) {
            std::cerr << "Invalid number of arguments for --sandbox"
                      << std::endl; // NOLINT
            std::cerr << program;
            exit(1); // NOLINT(concurrency-*)
        }

        std::string uid = sandbox[0];
        std::string algo_id = sandbox[1];

        std::replace(uid.begin(), uid.end(), ' ', '-');
        std::replace(algo_id.begin(), algo_id.end(), ' ', '-');

        return algorithm{uid, algo_id};
    };

    auto algo = get_sandbox_algo();
    bool dev_mode = program.get<bool>("--dev");
    bool bots_only = program.get<bool>("--bots-only");
    auto get_mode = [&]() -> mode {
        if (dev_mode)
            return mode::dev;
        if (algo.has_value())
            return mode::sandbox;
        if (bots_only)
            return mode::bots_only;
        return mode::normal;
    };

    return std::make_tuple(get_mode(), algo);
}

} // namespace config
} // namespace nutc
