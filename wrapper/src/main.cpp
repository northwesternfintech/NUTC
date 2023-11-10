#include "common.hpp"
#include "dev_mode/dev_mode.hpp"
#include "firebase/firebase.hpp"
#include "pywrapper/pywrapper.hpp"
#include "rabbitmq/rabbitmq.hpp"

#include <argparse/argparse.hpp>
#include <pybind11/pybind11.h>

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <tuple>

#ifndef NO_GIT_VERSION_TRACKING
#  include "git.h"
#endif

static std::tuple<uint8_t, std::string, bool>
process_arguments(int argc, const char** argv)
{
    argparse::ArgumentParser program(
        "NUTC Client", VERSION, argparse::default_arguments::help
    );

    program.add_argument("-D", "--dev")
        .help("Enable development features")
        .action([](const auto& /* unused */) {})
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

    program.add_argument("-V", "--version")
        .help("prints version information and exits")
        .action([&](const auto& /* unused */) {
            fmt::println("NUTC Client v{}", VERSION);
            exit(0); // NOLINT(concurrency-*)
        })
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
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        exit(1); // NOLINT(concurrency-*)
    }

    return std::make_tuple(
        verbosity, program.get<std::string>("--uid"), program.get<bool>("--dev")
    );
}

static void
log_build_info()
{
    log_i(main, "NUTC Client: Interface to the NUFT Trading Competition");

#ifndef NO_GIT_VERSION_TRACKING

    // Git info
    log_i(main, "Built from {} on {}", git_Describe(), git_Branch());
    log_d(main, "Commit: \"{}\" at {}", git_CommitSubject(), git_CommitDate());
    log_d(main, "Author: {} <{}>", git_AuthorName(), git_AuthorEmail());

    if (git_AnyUncommittedChanges())
        log_w(main, "Built from dirty commit!");

#endif
}

int
main(int argc, const char** argv)
{
    // Parse args
    auto [verbosity, uid, development_mode] = process_arguments(argc, argv);
    pybind11::scoped_interpreter guard{};

    // Start logging and print build info
    nutc::logging::init(verbosity, uid);
    log_build_info();
    log_i(main, "Starting NUTC Client for UID {}", uid);

    // Initialize the RMQ connection to the exchange
    nutc::rabbitmq::RabbitMQ conn(uid);

    std::optional<std::string> algo;
    if (development_mode) {
        algo = nutc::dev_mode::get_algo_from_file(uid);
    }
    else {
        algo = nutc::firebase::get_most_recent_algo(uid);
    }

    // Send message to exchange to let it know we successfully initialized
    bool published_init = conn.publishInit(uid, algo.has_value());
    if (!published_init) {
        log_e(main, "Failed to publish init message");
        return 1;
    }
    if (!algo.has_value()) {
        return 0;
    }
    conn.waitForStartTime();

    // Initialize the algorithm. For now, only designed for py
    nutc::pywrapper::create_api_module(conn.getMarketFunc(uid));
    nutc::pywrapper::run_code_init(algo.value());

    // Main event loop
    conn.handleIncomingMessages();
    return 0;
}
