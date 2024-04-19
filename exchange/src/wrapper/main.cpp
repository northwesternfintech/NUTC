#include "shared/config/config.h"
#include "shared/file_operations/file_operations.hpp"
#include "wrapper/firebase/firebase.hpp"
#include "wrapper/logging.hpp"
#include "wrapper/pywrapper/pywrapper.hpp"
#include "wrapper/rabbitmq/rabbitmq.hpp"

#include <argparse/argparse.hpp>
#include <pybind11/pybind11.h>

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>

struct wrapper_args {
    uint8_t verbosity;
    std::string uid;
    std::string algo_id;
    bool dev_mode;
    bool no_start_delay;
};

namespace {
wrapper_args
process_arguments(int argc, const char** argv)
{
    argparse::ArgumentParser program(
        "NUTC Wrapper", VERSION, argparse::default_arguments::help
    );

    program.add_argument("-D", "--dev")
        .help("Enable development features")
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

    program.add_argument("--no-start-delay")
        .help("Disable the start time delay. Start running immediately after "
              "initialization message")
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

    program.add_argument("-V", "--version")
        .help("prints version information and exits")
        .action([&](const auto& /* unused */) {
            fmt::println("NUTC wrapper v{}", VERSION);
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
        std::cerr << err.what() << std::endl; // NOLINT
        std::cerr << program;
        exit(1); // NOLINT(concurrency-*)
    }

    return {
        verbosity, program.get<std::string>("--uid"),
        program.get<std::string>("--algo_id"), program.get<bool>("--dev"),
        program.get<bool>("--no-start-delay")
    };
}
} // namespace

class NullBuffer : public std::streambuf {
public:
    NullBuffer() = default;
};

int
main(int argc, const char** argv)
{
    // Parse args
    auto [verbosity, uid, algo_id, development_mode, no_start_delay] =
        process_arguments(argc, argv);
    pybind11::scoped_interpreter guard{};

    if (!development_mode) {
        NullBuffer null_buffer;
        std::cout.rdbuf(&null_buffer);
        std::cerr.rdbuf(&null_buffer);
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);
    }

    // Start logging and print build info
    nutc::logging::init(verbosity);
    log_i(main, "Starting NUTC wrapper for UID {}", uid);

    // Initialize the RMQ connection to the exchange
    nutc::rabbitmq::RabbitMQHandler conn{uid};

    std::optional<std::string> algo{};
    if (development_mode) {
        log_i(main, "Running in development mode");
        algo = nutc::file_ops::read_file_content(algo_id);
    }
    else {
        algo = nutc::firebase::get_algo(uid, algo_id);
    }

    // Send message to exchange to let it know we successfully initialized
    bool published_init = conn.publish_init_message(uid, algo.has_value());
    if (!published_init) {
        throw std::runtime_error("Failed to publish init message to exchange");
        return 1;
    }
    if (!algo.has_value()) {
        return 0;
    }
    conn.wait_for_start_time(no_start_delay);

    // Initialize the algorithm. For now, only designed for py
    nutc::pywrapper::create_api_module(conn.market_order_func(uid));
    nutc::pywrapper::run_code_init(algo.value());

    // Main event loop
    conn.main_event_loop(uid);
    return 0;
}
