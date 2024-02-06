#include "algos/algo_manager.hpp"
#include "algos/dev_mode/dev_mode.hpp"
#include "algos/normal_mode/normal_mode.hpp"
#include "algos/sandbox_mode/sandbox_mode.hpp"
#include "bot_framework/bot_container_mapper.hpp"
#include "client_manager/client_manager.hpp"
#include "config.h"
#include "exchange/tick_manager/tick_manager.hpp"
#include "logging.hpp"
#include "matching/manager/engine_manager.hpp"
#include "process_spawning/spawning.hpp"
#include "rabbitmq/client_manager/RabbitMQClientManager.hpp"
#include "rabbitmq/connection_manager/RabbitMQConnectionManager.hpp"
#include "rabbitmq/consumer/RabbitMQConsumer.hpp"
#include "utils/logger/logger.hpp"

#include <argparse/argparse.hpp>
#include <csignal>

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

    program.add_argument("--bots-only")
        .help("No algos spawned in, just bots")
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
    bool bots_only = program.get<bool>("--bots-only");
    auto get_mode = [&]() -> Mode {
        if (dev_mode)
            return Mode::DEV;
        if (algo.has_value())
            return Mode::SANDBOX;
        if (bots_only)
            return Mode::BOTS_ONLY;
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
initialize_ticker(const std::string& ticker, float starting_price)
{
    using nutc::bots::BotContainerMapper;
    using nutc::engine_manager::EngineManager;
    using nutc::ticks::PRIORITY;
    using nutc::ticks::TickManager;

    auto& tick_manager = TickManager::get_instance();
    EngineManager::get_instance().add_engine(ticker);
    EngineManager::get_instance().set_initial_price(ticker, starting_price);

    auto& bot_container = BotContainerMapper::get_instance(ticker, starting_price);

    // Should run after stale order removal, so they can react to removed orders
    tick_manager.attach(&bot_container, PRIORITY::second);
}

int
main(int argc, const char** argv)
{
    std::signal(SIGINT, flush_log);

    using namespace nutc; // NOLINT(*)

    // Set up logging
    logging::init(quill::LogLevel::TraceL3);

    static constexpr uint16_t TICK_HZ = 5;
    nutc::ticks::TickManager::get_instance(TICK_HZ);

    initialize_ticker("A", 100);
    initialize_ticker("B", 200);
    initialize_ticker("C", 300);

    bots::BotContainerMapper::get_instance("A").add_mm_bot("1", 50000);
    bots::BotContainerMapper::get_instance("A").add_mm_bot("2", 50000);
    bots::BotContainerMapper::get_instance("A").add_mm_bot("3", 50000);

    auto& engine_manager = engine_manager::EngineManager::get_instance();
    ticks::TickManager::get_instance().attach(&engine_manager, ticks::PRIORITY::first);
    ticks::TickManager::get_instance().start();

    manager::ClientManager& users = manager::ClientManager::get_instance();

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
            break;
        case Mode::BOTS_ONLY:
            break;
    }

    client::spawn_all_clients(users);

    // Run exchange
    rabbitmq::RabbitMQClientManager::wait_for_clients(users, num_clients);
    rabbitmq::RabbitMQClientManager::send_start_time(users, CLIENT_WAIT_SECS);

    // Main event loop
    rabbitmq::RabbitMQConsumer::handle_incoming_messages(users, engine_manager);

    return 0;
}
