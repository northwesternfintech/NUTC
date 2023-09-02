#define CROW_MAIN
#include "common.hpp"
#include "git.h"
#include "lint/lint.hpp"

#include <argparse/argparse.hpp>
#include <crow/app.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>

static std::tuple<uint8_t>
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

    return std::make_tuple(verbosity);
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
    auto [verbosity] = process_arguments(argc, argv);

    // Start logging and print build info
    nutc::logging::init(verbosity);
    log_build_info();
    log_i(main, "Starting NUTC Linter");

    crow::SimpleApp app;

    CROW_ROUTE(app, "/")
    ([&](const crow::request& req) {
        if (!req.url_params.get("uid")) {
            log_e(main, "No uid provided");
            return crow::response(400);
        };
        if (!req.url_params.get("algo_id")) {
            log_e(main, "No algo_id provided");
            return crow::response(400);
        }
        std::streambuf* oldCoutStreamBuf = std::cout.rdbuf();
        std::stringstream ss;
        std::cout.rdbuf(ss.rdbuf());

        std::string uid = req.url_params.get("uid");
        std::string algo_id = req.url_params.get("algo_id");
        log_i(main, "Linting algo_id: {} for user: {}", algo_id, uid);
        std::string response = nutc::lint::lint(uid, algo_id);
        std::cout.rdbuf(oldCoutStreamBuf);
        nutc::client::set_lint_success(uid, algo_id, ss.str() + "\n");
        return crow::response(response);
    });

    app.port(8080).run();

    return 0;
}
