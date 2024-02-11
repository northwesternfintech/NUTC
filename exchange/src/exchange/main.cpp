#include "algos/algo_manager.hpp"
#include "algos/dev_mode/dev_mode.hpp"
#include "algos/normal_mode/normal_mode.hpp"
#include "algos/sandbox_mode/sandbox_mode.hpp"
#include "config.h"
#include "dashboard/dashboard.hpp"
#include "exchange/bots/bot_container.hpp"
#include "exchange/dashboard/state/global_metrics.hpp"
#include "exchange/tick_manager/tick_manager.hpp"
#include "logging.hpp"
#include "process_spawning/spawning.hpp"
#include "rabbitmq/client_manager/RabbitMQClientManager.hpp"
#include "rabbitmq/connection_manager/RabbitMQConnectionManager.hpp"
#include "rabbitmq/consumer/RabbitMQConsumer.hpp"
#include "tickers/manager/ticker_manager.hpp"
#include "traders/trader_manager.hpp"
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
print_file_contents(const std::string& filepath)
{
    std::ifstream file(filepath);

    if (!file) {
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::cout << line << std::endl; // NOLINT
    }
}

void
flush_log(int sig) // NOLINT(*)
{
    nutc::events::Logger::get_logger().flush();
    nutc::dashboard::Dashboard::get_instance().close();
    print_file_contents("logs/error_log.txt");
    std::exit(0);
}

// Initializes tick manager with brownian motion
void
initialize_ticker(const std::string& ticker, double starting_price)
{
    using nutc::dashboard::DashboardState;
    using nutc::engine_manager::EngineManager;
    using nutc::ticks::PRIORITY;
    using nutc::ticks::TickManager;

    auto& tick_manager = TickManager::get_instance();
    EngineManager::get_instance().add_engine(ticker, starting_price);

    nutc::bots::BotContainer& bot_container =
        EngineManager::get_instance().get_bot_container(ticker);

    // Should run after stale order removal, so they can react to removed orders
    tick_manager.attach(&bot_container, PRIORITY::second);

    DashboardState::get_instance().add_ticker(ticker, starting_price);

    auto& ticker_state = DashboardState::get_instance().get_ticker_state(ticker);
    tick_manager.attach(&ticker_state, PRIORITY::third);
}

// todo: please god clean this up
int
main(int argc, const char** argv)
{
    std::signal(SIGINT, flush_log);
    std::signal(SIGABRT, flush_log);

    using namespace nutc; // NOLINT(*)
    // Set up logging
    logging::init(quill::LogLevel::Info);

    static constexpr uint16_t TICK_HZ = 10;
    nutc::ticks::TickManager::get_instance(TICK_HZ);

    initialize_ticker("ETH", 100);
    initialize_ticker("BTC", 200);
    initialize_ticker("USD", 300);

    auto& dashboard = nutc::dashboard::Dashboard::get_instance();
    nutc::ticks::TickManager::get_instance().attach(
        &dashboard, nutc::ticks::PRIORITY::fourth
    );

    auto& engine_manager = engine_manager::EngineManager::get_instance();

    engine_manager.get_bot_container("ETH").add_mm_bots(
        {50000, 25000, 25000, 10000, 10000, 5000}
    );                                                            // NOLINT(*)
    engine_manager.get_bot_container("BTC").add_mm_bots({50000}); // NOLINT(*)
    engine_manager.get_bot_container("USD").add_mm_bots(
        {100000, 100000, 100000, 25000, 25000, 10000, 5000, 5000, 5000, 5000}
    ); // NOLINT(*)

    engine_manager.get_bot_container("ETH").add_retail_bots(10, 3, 10);
    engine_manager.get_bot_container("BTC").add_retail_bots(100, 30, 2);
    engine_manager.get_bot_container("USD").add_retail_bots(1000, 300, 100);

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

    while (mode == Mode::BOTS_ONLY) {
    } // spin forever, but keep rmq running. maybe remove this later?

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

    spawning::spawn_all_clients(users);

    // Run exchange
    rabbitmq::RabbitMQClientManager::wait_for_clients(users, num_clients);
    rabbitmq::RabbitMQClientManager::send_start_time(users, CLIENT_WAIT_SECS);

    // Main event loop
    rabbitmq::RabbitMQConsumer::handle_incoming_messages(users, engine_manager);

    return 0;
}
