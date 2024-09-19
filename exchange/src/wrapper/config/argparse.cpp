#include "argparse.hpp"

#include "common/config/config.h"

#include <argparse/argparse.hpp>
#include <fmt/format.h>

namespace nutc::wrapper {
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

    program.add_argument("-B", "--binary_algo")
        .help("Run a binary, compiled algorithm")
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

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

    bool dev_mode = program.get<bool>("--dev");
    auto trader_id = program.get<std::string>("--uid");

    auto algo_type = program.get<bool>("--binary_algo") ? common::AlgoLanguage::cpp
                                                        : common::AlgoLanguage::python;

    return {verbosity, trader_id, algo_type};
}
} // namespace nutc::wrapper
