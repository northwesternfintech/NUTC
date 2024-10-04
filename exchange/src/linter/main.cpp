#include <quill/LogLevel.h>
#define CROW_MAIN
#include "common/logging/logging.hpp"
#include "linter/config.h"
#include "linter/crow/crow.hpp"

#include <argparse/argparse.hpp>
#include <fmt/core.h>

#include <iostream>
#include <string>
#include <thread>
#include <tuple>

static void
process_arguments(int argc, const char** argv)
{
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
    nutc::logging::init(quill::LogLevel::Info);
    log_i(main, "Starting NUTC Linter");

    auto server_thread = nutc::crow::get_server_thread();

    server_thread.join();

    return 0;
}