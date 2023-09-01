#include "common.hpp"
#include "firebase/firebase.hpp"
#include "git.h"
#include "mock_api/mock_api.hpp"
#include "pywrapper/pywrapper.hpp"

#include <argparse/argparse.hpp>
#include <pybind11/pybind11.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <tuple>

static std::tuple<uint8_t, std::string, std::string>
process_arguments(int argc, const char** argv)
{
    argparse::ArgumentParser program(
        "NUTC Linter", VERSION, argparse::default_arguments::help
    );

    program.add_argument("--uid")
        .help("Set the user ID. Any spaces will be converted to dashes")
        .action([](const auto& value) {
            std::string uid = std::string(value);
            std::replace(uid.begin(), uid.end(), ' ', '-');
            return uid;
        })
        .required();

    program.add_argument("--algo_id")
        .help("Set the algorithm ID of the given user. Any spaces will be converted to "
              "dashes")
        .action([](const auto& value) {
            std::string id = std::string(value);
            std::replace(id.begin(), id.end(), ' ', '-');
            return id;
        })
        .required();

    program.add_argument("-V", "--version")
        .help("prints version information and exits")
        .action([&](const auto& /* unused */) {
            fmt::println("NUTC Linter v{}", VERSION);
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
        verbosity,
        program.get<std::string>("--uid"),
        program.get<std::string>("--algo_id")
    );
}

static void
log_build_info()
{
    log_i(main, "NUTC Linter: Linter for NUTC user-submitted algorithms");

    // Git info
    log_i(main, "Built from {} on {}", git_Describe(), git_Branch());
    log_d(main, "Commit: \"{}\" at {}", git_CommitSubject(), git_CommitDate());
    log_d(main, "Author: {} <{}>", git_AuthorName(), git_AuthorEmail());

    if (git_AnyUncommittedChanges())
        log_w(main, "Built from dirty commit!");
}

int
main(int argc, const char** argv)
{
    // Parse args
    auto [verbosity, uid, algo_id] = process_arguments(argc, argv);

    // Start logging and print build info
    nutc::logging::init(verbosity);
    log_build_info();
    log_i(main, "Starting NUTC Linter for UID {} and algorithm ID {}", uid, algo_id);

    std::optional<std::string> algoCode = nutc::client::get_most_recent_algo(uid);
    if (!algoCode.has_value()) {
        return 0;
    }

    pybind11::scoped_interpreter guard{};
    bool e = nutc::pywrapper::create_api_module(nutc::mock_api::getMarketFunc());
    if (!e) {
        log_e(main, "Failed to create API module");
        return 1;
    }

    std::optional<std::string> err = nutc::pywrapper::import_py_code(algoCode.value());
    if (err.has_value()) {
        log_e(main, "{}", err.value());
        return 1;
    }

    err = nutc::pywrapper::run_initialization();
    if (err.has_value()) {
        log_e(main, "{}", err.value());
        return 1;
    }

    err = nutc::pywrapper::trigger_callbacks();
    if (err.has_value()) {
        log_e(main, "{}", err.value());
        return 1;
    }

    return 0;
}
