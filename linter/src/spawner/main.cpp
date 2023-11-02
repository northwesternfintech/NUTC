#include "common.hpp"
#include "lint/lint.hpp"

#include <argparse/argparse.hpp>
#include <pybind11/pybind11.h>

#include <chrono>
#include <cstdlib>

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <tuple>

static std::tuple<uint8_t, std::string, std::string>
process_arguments(int argc, const char** argv)
{
    argparse::ArgumentParser program(
        "NUTC Linter Process Spawner", "N/A", argparse::default_arguments::help
    );

    program.add_argument("-U", "--uid")
        .help("set the user ID")
        .action([](const auto& value) {
            std::string uid = std::string(value);
            std::replace(uid.begin(), uid.end(), ' ', '-');
            return uid;
        })
        .required();

    program.add_argument("-A", "--algoid")
        .help("set the algorithm ID")
        .action([](const auto& value) {
            std::string algoid = std::string(value);
            std::replace(algoid.begin(), algoid.end(), ' ', '-');
            return algoid;
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
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        exit(1); // NOLINT(concurrency-*)
    }

    return std::make_tuple(
        verbosity,
        program.get<std::string>("--uid"),
        program.get<std::string>("--algoid")
    );
}

int
main(int argc, const char** argv)
{
    // Parse args
    auto [verbosity, uid, algoid] = process_arguments(argc, argv);

    // Start logging and print build info
    nutc::logging::init(verbosity);

    // Move a string stream from the loop to this process
    std::stringstream ss;

    // Watchdog to kill after 120s
    std::thread timeout_thread([&ss = ss, &algoid = algoid, &uid = uid]() {
        std::this_thread::sleep_for(std::chrono::seconds(120));

        log_e(
            main,
            "Timeout reached. Exiting process. Failed lint for algoid {} and uid {}.",
            algoid,
            uid
        );
        ss << "[linter] FAILED to lint algo_id " << algoid << " for uid " << uid << "\n";

        nutc::client::set_lint_failure(uid, algoid, ss.str() + "Failure!\n");
        std::exit(1);
    });
    timeout_thread.detach();

    // Log this event
    log_i(main, "Linting algo_id: {} for user: {}", algoid, uid);
    ss << "[linter] starting to lint algo_id " << algoid << " for uid " << uid << "\n";

    // Initialize py
    pybind11::initialize_interpreter();

    // Actually lint the algo and set success
    std::string response = nutc::lint::lint(uid, algoid, ss);

    log_i(main, "Finished linting algo_id: {} for user: {}", algoid, uid);
    ss << "[linter] exited linting process and finished linting algo_id " << algoid << " for uid " << uid << "\n";

    nutc::client::set_lint_success(uid, algoid, ss.str() + "Success!\n");

    // Stop py
    pybind11::finalize_interpreter();

    // Done!
    return 0;
}
