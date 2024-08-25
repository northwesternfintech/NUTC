#include "argparse.hpp"

#include "shared/config/config.h"

namespace nutc::exchange {

mode
process_arguments(int argc, const char** argv)
{
    argparse::ArgumentParser program(
        "NUTC", NUTC_VERSION, argparse::default_arguments::help
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

    program.add_argument("--sandbox")
        .help("Start crow server to support sandbox")
        .action([](const auto& /* unused */) {})
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

    program.add_argument("-V", "--version")
        .help("prints version information and exits")
        .action([&](const auto& /* unused */) {
            fmt::println("NUTC24  v{}", NUTC_VERSION);
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

    if (program.get<bool>("--dev"))
        return mode::dev;
    if (program.get<bool>("--bots-only"))
        return mode::bots_only;
    if (program.get<bool>("--sandbox"))
        return mode::sandbox;
    return mode::normal;
}

} // namespace nutc::exchange
