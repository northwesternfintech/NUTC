#include "algos/algo_manager.hpp"
#include "algos/dev_mode/dev_mode.hpp"
#include "algos/normal_mode/normal_mode.hpp"
#include "algos/sandbox_mode/sandbox_mode.hpp"
#include "bot_framework/bot_container.hpp"
#include "client_manager/client_manager.hpp"
#include "config.h"
#include "exchange/tick_manager/tick_manager.hpp"
#include "logging.hpp"
#include "matching/manager/engine_manager.hpp"
#include "process_spawning/spawning.hpp"
#include "rabbitmq/client_manager/RabbitMQClientManager.hpp"
#include "rabbitmq/connection_manager/RabbitMQConnectionManager.hpp"
#include "rabbitmq/consumer/RabbitMQConsumer.hpp"
#include "rabbitmq/order_handler/RabbitMQOrderHandler.hpp"
#include "utils/logger/logger.hpp"

#include <argparse/argparse.hpp>
#include <signal.h>

#include <iostream>
#include <string>

#include <rabbitmq-c/amqp.h>

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
        return Mode::NORMAL;
    };

    return std::make_tuple(get_mode(), algo);
}

void
flush_log(int sig) // NOLINT(*)
{
    nutc::events::Logger::get_logger().flush();
    std::exit(0);
}

// Initializes tick manager with brownian motion
void
initialize_tick_manager()
{
    auto& tick_listener = nutc::bots::BotContainer::get_instance();
    auto& tick_manager = nutc::ticks::TickManager::get_instance(10);
    tick_manager.attach(&tick_listener);
    tick_manager.start();
}

int
main(int argc, const char** argv)
{
    using namespace nutc; // NOLINT(*)

    initialize_tick_manager();

    // Set up logging
    logging::init(quill::LogLevel::TraceL3);

    std::signal(SIGINT, flush_log);

    manager::ClientManager users;
    nutc::engine_manager::Manager engine_manager;

    auto [mode, sandbox] = process_arguments(argc, argv);

    auto& rmq_conn = rabbitmq::RabbitMQConnectionManager::get_instance();

    // Connect to RabbitMQ
    if (!rmq_conn.connected_to_rabbitmq()) {
        log_e(rabbitmq, "Failed to initialize connection");
        return 1;
    }

    size_t num_clients{};
    using algo_mgmt::AlgoManager;

    auto initialize_dev_mode = [&]() {
        log_t1(main, "Initializing NUTC in development mode");
        using algo_mgmt::DevModeAlgoManager;

        DevModeAlgoManager manager = DevModeAlgoManager(DEBUG_NUM_USERS);
        num_clients = AlgoManager::initialize_algo_management(manager, users);
    };

    // Weird name because of shadowing
    auto initialize_sandbox_mode = [&, &sandbox = sandbox]() {
        log_t1(main, "Initializing NUTC in sandbox mode");
        using algo_mgmt::SandboxAlgoManager;

        auto& [uid, algo_id] = sandbox.value(); // NOLINT (unchecked-*)
        SandboxAlgoManager manager = SandboxAlgoManager(uid, algo_id);
        num_clients = AlgoManager::initialize_algo_management(manager, users);
    };

    auto initialize_normal_mode = [&]() {
        log_t1(main, "Initializing NUTC in normal mode");
        using algo_mgmt::NormalModeAlgoManager;

        NormalModeAlgoManager manager = NormalModeAlgoManager();
        num_clients = AlgoManager::initialize_algo_management(manager, users);
    };

    switch (mode) {
        case Mode::DEV:
            initialize_dev_mode();
            break;
        case Mode::SANDBOX:
            initialize_sandbox_mode();
            break;
        case Mode::NORMAL:
            initialize_normal_mode();
    }

    client::spawn_all_clients(users);

    engine_manager.add_engine("A");
    engine_manager.add_engine("B");
    engine_manager.add_engine("C");

    // Run exchange
    rabbitmq::RabbitMQClientManager::wait_for_clients(users, num_clients);
    rabbitmq::RabbitMQClientManager::send_start_time(users, CLIENT_WAIT_SECS);
    rabbitmq::RabbitMQOrderHandler::add_liquidity_to_ticker(
        users, engine_manager, "A", 1000, 100
    );
    rabbitmq::RabbitMQOrderHandler::add_liquidity_to_ticker(
        users, engine_manager, "B", 2000, 200
    );
    rabbitmq::RabbitMQOrderHandler::add_liquidity_to_ticker(
        users, engine_manager, "C", 3000, 300
    );

    // Main event loop
    rabbitmq::RabbitMQConsumer::handle_incoming_messages(users, engine_manager);

    return 0;
}
