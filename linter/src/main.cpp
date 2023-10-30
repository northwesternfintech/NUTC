#define CROW_MAIN
#include "common.hpp"

#include <argparse/argparse.hpp>
#include <crow/app.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
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

void
spawn_client(const std::string& uid, std::string& algoid)
{
    std::replace(algoid.begin(), algoid.end(), '-', ' ');
    pid_t pid = fork();
    if (pid == 0) {
        std::vector<std::string> args = {
            "NUTC-linter-spawner", "--uid", uid, "--algoid", algoid
        };

        std::vector<char*> c_args;
        for (auto& arg : args)
            c_args.push_back(arg.data());
        c_args.push_back(nullptr);

        execvp(c_args[0], c_args.data());

        log_e(linting, "Failed to lint algoid {} for uid {}", algoid, uid);

        exit(1);
    }
    else if (pid < 0) {
        log_e(linting, "Failed to fork");
        exit(1);
    }
}

int
main(int argc, const char** argv)
{
    // Parse args
    auto [verbosity] = process_arguments(argc, argv);

    // Start logging and print build info
    nutc::logging::init(verbosity);
    log_i(main, "Starting NUTC Linter");

    std::thread server_thread([]() {
        crow::SimpleApp app;
        CROW_ROUTE(app, "/")
        ([&](const crow::request& req) {
            log_i(main, "Registered");

            if (!req.url_params.get("uid")) {
                log_e(main, "No uid provided");
                return crow::response(400);
            };
            if (!req.url_params.get("algo_id")) {
                log_e(main, "No algo_id provided");
                return crow::response(400);
            }
            std::string uid = req.url_params.get("uid");
            std::string algo_id = req.url_params.get("algo_id");
            
            spawn_client(uid, algo_id);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            return crow::response();
        });
        app.port(8080).run();
    });

    server_thread.join();

    return 0;
}
