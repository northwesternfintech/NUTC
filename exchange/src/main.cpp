#include "client_manager/client_manager.hpp"
#include "config.h"
#include "local_algos/dev_mode.hpp"
#include "local_algos/sandbox.hpp"
#include "logging.hpp"
#include "matching/engine/engine.hpp"
#include "process_spawning/spawning.hpp"
#include "rabbitmq/rabbitmq.hpp"

#include <argparse/argparse.hpp>

#include <iostream>
#include <string>

#include <rabbitmq-c/amqp.h>

namespace rmq = nutc::rabbitmq;

nutc::manager::ClientManager users;
nutc::engine_manager::Manager engine_manager;

struct algorithm {
    std::string uid;
    std::string algo_id;
};

static std::tuple<Mode, std::optional<algorithm>>
process_arguments(int argc, const char** argv)
{
    argparse::ArgumentParser program(
        "NUTC24", VERSION, argparse::default_arguments::help
    );

    program.add_argument("-D", "--dev")
        .help("Enable development features")
        .action([](const auto& /* unused */) {})
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

    program.add_argument("-S", "--sandbox").help("Provide a sandbox algo id").nargs(2);

    program.add_argument("-V", "--version")
        .help("prints version information and exits")
        .action([&](const auto& /* unused */) {
            fmt::println("NUTC24  v{}", VERSION);
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

    std::optional<algorithm> algo = std::nullopt;
    if (program.is_used("--sandbox")) {
        auto sandbox = program.get<std::vector<std::string>>("--sandbox");
        if (sandbox.size() != 2) {
            std::cerr << "Invalid number of arguments for --sandbox" << std::endl;
            std::cerr << program;
            exit(1); // NOLINT(concurrency-*)
        }

        std::string uid = sandbox[0];
        std::string algo_id = sandbox[1];

        std::replace(uid.begin(), uid.end(), ' ', '-');
        std::replace(algo_id.begin(), algo_id.end(), ' ', '-');

        algo = algorithm{uid, algo_id};
    }

    bool dev_mode = program.get<bool>("--dev");
    auto get_mode = [&]() -> Mode {
        if (dev_mode)
            return Mode::DEV;
        if (algo.has_value())
            return Mode::SANDBOX;
        return Mode::PROD;
    };

    return std::make_tuple(get_mode(), algo);
}

void
handle_sigint(int sig)
{
    log_i(rabbitmq, "Caught SIGINT, closing connection");
    sleep(1);
    exit(sig);
}

int
main(int argc, const char** argv)
{
    auto [mode, sandbox] = process_arguments(argc, argv);

    // Set up logging
    nutc::logging::init(quill::LogLevel::TraceL3);

    // Initialize signal handler
    signal(SIGINT, handle_sigint);

    auto& rmq_conn = rmq::RabbitMQConnectionManager::get_instance();

    // Connect to RabbitMQ
    if (!rmq_conn.connected_to_rabbitmq()) {
        log_e(rabbitmq, "Failed to initialize connection");
        return 1;
    }

    if (mode == Mode::DEV) {
        log_t1(main, "Initializing NUTC in development mode");
        nutc::dev_mode::create_mt_algo_files(DEBUG_NUM_USERS);
    }
    else if (mode == Mode::SANDBOX) {
        log_t1(main, "Initializing NUTC in sandbox node");
        nutc::sandbox::create_sandbox_algo_files();
        auto& [uid, algo_id] = sandbox.value();
        users.add_client(uid, algo_id, false);
    }

    size_t num_clients = nutc::client::initialize(users, mode);

    engine_manager.add_engine("A");
    engine_manager.add_engine("B");
    engine_manager.add_engine("C");

    // Run exchange
    rmq::RabbitMQClientManager::wait_for_clients(users, num_clients);
    rmq::RabbitMQClientManager::send_start_time(users, CLIENT_WAIT_SECS);
    rmq::RabbitMQOrderHandler::add_liquidity_to_ticker(
        users, engine_manager, "A", 1000, 100
    );
    rmq::RabbitMQOrderHandler::add_liquidity_to_ticker(
        users, engine_manager, "B", 2000, 200
    );
    rmq::RabbitMQOrderHandler::add_liquidity_to_ticker(
        users, engine_manager, "C", 3000, 300
    );
    rmq::RabbitMQConsumer::handle_incoming_messages(users, engine_manager);

    return 0;
}
