#include "common/config/config.hpp"

#include <quill/LogLevel.h>
#define CROW_MAIN
#include "common/logging/logging.hpp"
#include "linter/crow/crow.hpp"

#include <argparse/argparse.hpp>
#include <fmt/core.h>

#include <iostream>
#include <string>

static void
process_arguments(int argc, const char** argv)
{
    using nutc::common::NUTC_VERSION;

    argparse::ArgumentParser program(
        "NUTC Linter", NUTC_VERSION, argparse::default_arguments::help
    );

    program.add_argument("-V", "--version")
        .help("prints version information and exits")
        .action([&](const auto& /* unused */) {
            fmt::println("NUTC Linter v{}", NUTC_VERSION);
            exit(0); // NOLINT(concurrency-*)
        })
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        exit(1); // NOLINT(concurrency-*)
    }
}

int
main(int argc, const char** argv)
{
    process_arguments(argc, argv);

    // Start logging and print the build info
    nutc::common::logging_init("linter.log", quill::LogLevel::Info);
    log_i(main, "Starting NUTC Linter");

    nutc::linter::get_crow_app().port(18081).run();

    return 0;
}
