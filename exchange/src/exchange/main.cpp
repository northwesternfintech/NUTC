#include "config.h"
#include "dashboard/dashboard.hpp"
#include "exchange/algos/algo_manager.hpp"
#include "exchange/bots/bot_container.hpp"
#include "exchange/concurrency/pin_threads.hpp"
#include "exchange/config/argparse.hpp"
#include "exchange/dashboard/state/global_metrics.hpp"
#include "exchange/tick_manager/tick_manager.hpp"
#include "logging.hpp"
#include "process_spawning/spawning.hpp"
#include "rabbitmq/connection_manager/RabbitMQConnectionManager.hpp"
#include "rabbitmq/consumer/RabbitMQConsumer.hpp"
#include "rabbitmq/trader_manager/RabbitMQTraderManager.hpp"
#include "shared/file_operations/file_operations.hpp"
#include "tickers/manager/ticker_manager.hpp"
#include "traders/trader_manager.hpp"
#include "utils/logger/logger.hpp"

#include <csignal>

#include <string>

#include <rabbitmq-c/amqp.h>

namespace {
void
flush_log(int)
{
    nutc::events::Logger::get_logger().flush();

    nutc::dashboard::Dashboard::get_instance().close();

    nutc::file_ops::print_file_contents("logs/error_log.txt");
    std::exit(0); // NOLINT(concurrency-*)
}

// Initializes tick manager with brownian motion
void
initialize_ticker(const std::string& ticker, double starting_price)
{
    using nutc::engine_manager::EngineManager;
    using nutc::ticks::PRIORITY;
    using nutc::ticks::TickManager;

    auto& tick_manager = TickManager::get_instance();
    EngineManager::get_instance().add_engine(ticker, starting_price);

    nutc::bots::BotContainer& bot_container =
        EngineManager::get_instance().get_bot_container(ticker);

    // Should run after stale order removal, so they can react to removed orders
    tick_manager.attach(&bot_container, PRIORITY::second, "Bot Engine");

    nutc::dashboard::DashboardState::get_instance().add_ticker(ticker, starting_price);
}

void
initialize_dashboard()
{
    auto& dashboard = nutc::dashboard::Dashboard::get_instance();
    nutc::ticks::TickManager::get_instance().attach(
        &dashboard, nutc::ticks::PRIORITY::third, "Dashboard Manager"
    );
}
} // namespace

int
main(int argc, const char** argv)
{
    std::signal(SIGINT, flush_log);
    std::signal(SIGABRT, flush_log);

    using namespace nutc; // NOLINT(*)
    using Mode = util::Mode;

    manager::TraderManager& users = manager::TraderManager::get_instance();

    auto [mode, sandbox] = config::process_arguments(argc, argv);

    if (mode != Mode::BOTS_ONLY) {
        auto& rmq_conn = rabbitmq::RabbitMQConnectionManager::get_instance();
        rmq_conn.initialize_connection();

        // Connect to RabbitMQ
        if (!rmq_conn.connected_to_rabbitmq()) {
            log_e(rabbitmq, "Failed to initialize connection");
            return 1;
        }
    }

    auto algo_mgr = algo_mgmt::AlgoManager::get_algo_mgr(mode, sandbox);
    algo_mgr->initialize_algo_management(users);
    size_t num_clients = algo_mgr->get_num_clients();

    spawning::spawn_all_clients(users);

    // Set up logging
    logging::init(quill::LogLevel::Info);

    // Run exchange
    rabbitmq::RabbitMQTraderManager::wait_for_clients(users, num_clients);
    rabbitmq::RabbitMQTraderManager::send_start_time(users, CLIENT_WAIT_SECS);

    static constexpr uint16_t TICK_HZ = 60;
    nutc::ticks::TickManager::get_instance(TICK_HZ);

    initialize_ticker("ETH", 100);
    initialize_ticker("BTC", 200);
    initialize_ticker("LTC", 300);

    initialize_dashboard();

    auto& engine_manager = engine_manager::EngineManager::get_instance();

    engine_manager.get_bot_container("ETH").add_bots<bots::MarketMakerBot>(
        100000, 10000, 5
    );
    engine_manager.get_bot_container("BTC").add_bots<bots::MarketMakerBot>(
        25000, 5000, 10
    );
    engine_manager.get_bot_container("LTC").add_bots<bots::MarketMakerBot>(
        100000, 25000, 3
    );
    engine_manager.get_bot_container("ETH").add_bots<bots::RetailBot>(10, 3, 200);
    engine_manager.get_bot_container("BTC").add_bots<bots::RetailBot>(100, 5, 500);
    engine_manager.get_bot_container("LTC").add_bots<bots::RetailBot>(100, 10, 100);

    ticks::TickManager::get_instance().attach(
        &engine_manager, ticks::PRIORITY::first, "Matching Engine"
    );
    ticks::TickManager::get_instance().start();

    concurrency::pin_to_core(0, "main");

    // Main event loop
    if (mode == Mode::BOTS_ONLY) {
        while (true) {}
    }
    else {
        rabbitmq::RabbitMQConsumer::handle_incoming_messages(engine_manager);
    }

    return 0;
}
