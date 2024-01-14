#include "algos/dev_mode/dev_mode.hpp"
#include "algos/management_interface.hpp"
#include "algos/normal_mode/normal_mode.hpp"
#include "algos/sandbox_mode/sandbox_mode.hpp"
#include "client_manager/client_manager.hpp"
#include "config.h"
#include "logging.hpp"
#include "matching/manager/engine_manager.hpp"
#include "process_spawning/spawning.hpp"
#include "rabbitmq/connection_manager/RabbitMQConnectionManager.hpp"
#include "rabbitmq/client_manager/RabbitMQClientManager.hpp"
#include "rabbitmq/order_handler/RabbitMQOrderHandler.hpp"
#include "rabbitmq/consumer/RabbitMQConsumer.hpp"

#include <argparse/argparse.hpp>

#include <iostream>
#include <string>

#include <rabbitmq-c/amqp.h>

namespace rmq = nutc::rabbitmq;

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

size_t
initialize_algo_manager(
    nutc::algo_mgmt::AlgoManager& algo_manager,
    nutc::manager::ClientManager& client_manager
)
{
    algo_manager.initialize_files();
    algo_manager.initialize_client_manager(client_manager);
    return algo_manager.get_num_clients();
}

int
main(int argc, const char** argv)
{
    nutc::manager::ClientManager users;
    auto [mode, sandbox] = process_arguments(argc, argv);

    // Set up logging
    nutc::logging::init(quill::LogLevel::TraceL3);

    auto& rmq_conn = rmq::RabbitMQConnectionManager::get_instance();

    // Connect to RabbitMQ
    if (!rmq_conn.connected_to_rabbitmq()) {
        log_e(rabbitmq, "Failed to initialize connection");
        return 1;
    }

    size_t num_clients{};

    if (mode == Mode::DEV) {
        log_t1(main, "Initializing NUTC in development mode");
        nutc::algo_mgmt::DevModeAlgoManager manager =
            nutc::algo_mgmt::DevModeAlgoManager(DEBUG_NUM_USERS);
        num_clients = initialize_algo_manager(manager, users);
    }
    else if (mode == Mode::SANDBOX) {
        log_t1(main, "Initializing NUTC in sandbox mode");
        auto& [uid, algo_id] = sandbox.value();
        nutc::algo_mgmt::SandboxAlgoManager manager =
            nutc::algo_mgmt::SandboxAlgoManager(uid, algo_id);
        num_clients = initialize_algo_manager(manager, users);
    }
    else if (mode == Mode::PROD) {
        log_t1(main, "Initializing NUTC in normal mode");
        nutc::algo_mgmt::NormalModeAlgoManager manager =
            nutc::algo_mgmt::NormalModeAlgoManager();
        num_clients = initialize_algo_manager(manager, users);
    }

    nutc::client::spawn_all_clients(users);

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
