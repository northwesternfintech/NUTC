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

namespace {
struct wrapper_args {
    const uint8_t VERBOSITY;
    const std::string USER_ID;
    const std::string ALGO_ID;
};

wrapper_args
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

    return {
        verbosity,
        program.get<std::string>("--uid"),
        program.get<std::string>("--algoid")
    };
}
} // namespace

int
main(int argc, const char** argv)
{
    // Parse args
    auto args = process_arguments(argc, argv);
    auto [verbosity, uid, algoid] = args;

    // Start logging and print build info
    nutc::logging::init(args.VERBOSITY);

    // Move a string stream from the loop to this process
    std::stringstream ss;
    bool has_exited = false;

    // Watchdog to kill after LINT_AUTO_TIMEOUT_SECONDS (default 120)
    std::thread timeout_thread([&ss, &has_exited, args]() {
        auto algoid = args.ALGO_ID;
        auto uid = args.USER_ID;
        auto start_time = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start_time
               < std::chrono::seconds(LINT_AUTO_TIMEOUT_SECONDS)) {
            if (has_exited) {
                log_i(
                    timeout_watchdog,
                    "Algo id {} for uid {} has completed. Exiting process-level "
                    "watchdog.",
                    algoid,
                    uid
                );
                return;
            }
        }

        log_e(
            timeout_watchdog,
            "Internal timeout reached ({}s). Exiting process. Failed lint for algoid "
            "{} and uid "
            "{}.",
            LINT_AUTO_TIMEOUT_SECONDS,
            algoid,
            uid
        );
        ss << fmt::format("[linter] FAILED to lint algo_id {} for uid {}", algoid, uid)
           << "\n";

        nutc::client::set_lint_failure(
            uid,
            algoid,
            ss.str()
                + fmt::format(
                    "\nYour code did not execute within %d seconds. Check all "
                    "functions to see if you have an infinite loop or infinite "
                    "recursion.\n\nIf you continue to experience this error, reach out "
                    "to #nuft-support.\n",
                    LINT_AUTO_TIMEOUT_SECONDS
                )
        );
    });

    // Log this event
    log_i(timeout_watchdog, "Linting algo_id: {} for user: {}", algoid, uid);
    ss << fmt::format("[linter] starting to lint algo_id {} for uid {}", algoid, uid)
       << "\n";

    // Initialize py
    pybind11::initialize_interpreter();

    // Actually lint the algo and set success
    bool lint_success = nutc::lint::lint(uid, algoid, ss);

    log_i(
        timeout_watchdog,
        "Finished linting algo_id: {} for user: {} with success {}",
        algoid,
        uid,
        lint_success
    );
    ss << fmt::format(
        "[linter] exited linting process and finished linting {} for uid {}",
        algoid,
        uid
    ) << "\n";

    // Stop py
    pybind11::finalize_interpreter();
    has_exited = true;
    timeout_thread.join();

    // Done!
    return 0;
}
