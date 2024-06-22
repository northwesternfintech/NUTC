#include "shared/config/config.h"
#include "shared/file_operations/file_operations.hpp"
#include "shared/firebase/firebase.hpp"
#include "wrapper/logging.hpp"
#include "wrapper/messaging/comms.hpp"
#include "wrapper/pywrapper/pywrapper.hpp"
#include "wrapper/resource_limits.hpp"

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
};

namespace {
wrapper_args
process_arguments(int argc, const char** argv)
{
    argparse::ArgumentParser program(
        "NUTC Wrapper", NUTC_VERSION, argparse::default_arguments::help
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

    program.add_argument("-V", "--version")
        .help("prints version information and exits")
        .action([&](const auto& /* unused */) {
            fmt::println("NUTC wrapper v{}", NUTC_VERSION);
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
        verbosity,
        program.get<std::string>("--uid"),
        program.get<std::string>("--algo_id"),
        program.get<bool>("--dev"),
    };
}
} // namespace

// We stop the exchange with sigint. The wrapper should exit gracefully
void
catch_sigint(int)
{
    // Wait until we're forced to terminate
    while (true) {}
}

int
main(int argc, const char** argv)
{
    std::signal(SIGINT, catch_sigint);
    using comms = nutc::comms::ExchangeProxy;
    auto [verbosity, uid, algo_id, development_mode] = process_arguments(argc, argv);
    pybind11::scoped_interpreter guard{};

    nutc::limits::set_memory_limit(1024);

    comms exchange_conn{};
    algorithm_t algorithm = exchange_conn.consume_algorithm();
    std::string algorithm_str = algorithm.algorithm_content_str;

    std::string trader_id = nutc::util::trader_id(uid, algo_id);
    if (development_mode) {
        trader_id = algo_id;
    }

    // Send message to exchange to let it know we successfully initialized
    comms::publish_init_message();
    if (algorithm_str == "") {
        return 1;
    }
    comms::wait_for_start_time();

    nutc::pywrapper::create_api_module(exchange_conn.market_order_func());
    nutc::pywrapper::run_code_init(algorithm_str);

    exchange_conn.main_event_loop(trader_id);
    return 0;
}
