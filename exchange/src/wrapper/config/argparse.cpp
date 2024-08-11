#include "argparse.hpp"

#include "shared/config/config.h"

#include <argparse/argparse.hpp>
#include <fmt/format.h>

namespace nutc::config {
wrapper_args
process_arguments(int argc, const char** argv)
{
    argparse::ArgumentParser program(
        "NUTC Wrapper", NUTC_VERSION, argparse::default_arguments::help
    );

    program.add_argument("-D", "--dev")
        .help("Pull algorithm from localhost")
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

    program.add_argument("-U", "--uid")
        .help("set the user ID")
        .action([](const auto& value) {
            std::string uid = std::string(value);
            std::replace(uid.begin(), uid.end(), ' ', '-');
            return uid;
        })
        .required();

    program.add_argument("-A", "--algo_id")
        .help("set the algo ID")
        .action([](const auto& value) {
            std::string algo_id = std::string(value);
            std::replace(algo_id.begin(), algo_id.end(), ' ', '-');
            return algo_id;
        })
        .required();

    uint8_t verbosity = 0;
    program.add_argument("-v", "--verbose")
        .help("increase output verbosity")
        .action([&](const auto& /* unused */) { ++verbosity; })
        .append()
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

    return {
        verbosity,
        program.get<std::string>("--uid"),
        program.get<std::string>("--algo_id"),
        program.get<bool>("--dev"),
    };
}
} // namespace nutc::config
