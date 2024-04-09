#include "dashboard/dashboard.hpp"
#include "exchange/algos/algo_manager.hpp"
#include "exchange/bots/bot_container.hpp"
#include "exchange/bots/bot_types/market_maker.hpp"
#include "exchange/concurrency/pin_threads.hpp"
#include "exchange/config/argparse.hpp"
#include "exchange/dashboard/state/global_metrics.hpp"
#include "exchange/tick_manager/tick_manager.hpp"
#include "logging.hpp"
#include "process_spawning/spawning.hpp"
#include "rabbitmq/connection_manager/RabbitMQConnectionManager.hpp"
#include "rabbitmq/consumer/RabbitMQConsumer.hpp"
#include "rabbitmq/trader_manager/RabbitMQTraderManager.hpp"
#include "shared/config/config_loader.hpp"
#include "shared/file_operations/file_operations.hpp"
#include "shared/util.hpp"
#include "tickers/manager/ticker_manager.hpp"
#include "traders/trader_manager.hpp"
#include "utils/logger/logger.hpp"

#include <csignal>

#include <string>

#include <rabbitmq-c/amqp.h>

namespace {
using namespace nutc; // NOLINT

void
flush_log(int)
{
    events::Logger::get_logger().flush();

    dashboard::Dashboard::get_instance().close();

    file_ops::print_file_contents("logs/error_log.txt");
    std::exit(0); // NOLINT(concurrency-*)
}

// Initializes tick manager with brownian motion
void
initialize_indiv_ticker(const std::string& ticker, double starting_price)
{
    using engine_manager::EngineManager;
    using ticks::PRIORITY;
    using ticks::TickManager;

    auto& tick_manager = TickManager::get_instance();
    EngineManager::get_instance().add_engine(ticker, starting_price);

    bots::BotContainer& bot_container =
        EngineManager::get_instance().get_bot_container(ticker);

    // Should run after stale order removal, so they can react to removed orders
    tick_manager.attach(&bot_container, PRIORITY::second, "Bot Engine");

    dashboard::DashboardState::get_instance().add_ticker(ticker, starting_price);
}

void
initialize_tickers()
{
    const auto& tickers = config::Config::get_instance().get_tickers();
    for (const auto& ticker : tickers) {
        initialize_indiv_ticker(ticker.TICKER, ticker.STARTING_PRICE);
    }
}

void
initialize_dashboard()
{
    auto& dashboard = dashboard::Dashboard::get_instance();
    ticks::TickManager::get_instance().attach(
        &dashboard, ticks::PRIORITY::third, "Dashboard Manager"
    );
}

void
initialize_bots()
{
    auto bots = config::Config::get_instance().get_bots();
    auto& engine_manager = engine_manager::EngineManager::get_instance();
    for (const auto& bot : bots) {
        auto& container = engine_manager.get_bot_container(bot.ASSOC_TICKER);

        if (bot.TYPE == config::BotType::market_maker) {
            container.add_bots<bots::MarketMakerBot>(
                bot.AVERAGE_CAPITAL, bot.STD_DEV_CAPITAL, bot.NUM_BOTS
            );
        }
        else if (bot.TYPE == config::BotType::retail) {
            container.add_bots<bots::RetailBot>(
                bot.AVERAGE_CAPITAL, bot.STD_DEV_CAPITAL, bot.NUM_BOTS
            );
        }
    }
    // TODO(stevenewald): should this be somewhere else?
    ticks::TickManager::get_instance().attach(
        &engine_manager, ticks::PRIORITY::first, "Matching Engine"
    );
}

void
initialize_wrappers()
{
    manager::TraderManager& users = manager::TraderManager::get_instance();
    rabbitmq::RabbitMQConnectionManager::get_instance().initialize_connection();

    spawning::spawn_all_clients(users);

    rabbitmq::RabbitMQTraderManager::wait_for_clients(users);
    size_t wait_secs = config::Config::get_instance().constants().WAIT_SECS;
    rabbitmq::RabbitMQTraderManager::send_start_time(users, wait_secs);
}

void
start_tick_manager()
{
    auto tick_hz = config::Config::get_instance().constants().TICK_HZ;
    ticks::TickManager::get_instance().start(tick_hz);
}

void
initialize_algos(const auto& mode, const auto& sandbox)
{
    manager::TraderManager& users = manager::TraderManager::get_instance();
    auto algo_mgr = algo_mgmt::AlgoManager::get_algo_mgr(mode, sandbox);
    algo_mgr->initialize_algo_management(users);
}

void
blocking_event_loop(const auto& mode)
{
    if (mode == util::Mode::BOTS_ONLY) {
        while (true) {}
    }
    else {
        auto& eng_mgr = engine_manager::EngineManager::get_instance();
        rabbitmq::RabbitMQConsumer::handle_incoming_messages(eng_mgr);
    }
}
} // namespace

int
main(int argc, const char** argv)
{
    using namespace nutc; // NOLINT(*)
    logging::init(quill::LogLevel::Info);

    std::signal(SIGINT, flush_log);
    std::signal(SIGABRT, flush_log);

    auto [mode, sandbox] = config::process_arguments(argc, argv);

    // Algos must init before wrappers
    initialize_algos(mode, sandbox);

    if (mode != util::Mode::BOTS_ONLY)
        initialize_wrappers();

    initialize_tickers();
    initialize_bots();
    initialize_dashboard();
    start_tick_manager();

    concurrency::pin_to_core(0, "main");

    blocking_event_loop(mode);
    return 0;
}
