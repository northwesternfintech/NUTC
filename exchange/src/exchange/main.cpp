#include "algos/algo_manager.hpp"
#include "dashboard/dashboard.hpp"
#include "exchange/algos/algo_manager.hpp"
#include "exchange/bots/bot_container.hpp"
#include "exchange/bots/bot_types/market_maker.hpp"
#include "exchange/concurrency/pin_threads.hpp"
#include "exchange/config/argparse.hpp"
#include "exchange/dashboard/state/global_metrics.hpp"
#include "exchange/tick_scheduler/tick_scheduler.hpp"
#include "logging.hpp"
#include "process_spawning/spawning.hpp"
#include "rabbitmq/connection_handler/rmq_connection_handler.hpp"
#include "rabbitmq/consumer/rmq_consumer.hpp"
#include "rabbitmq/trader_manager/rmq_wrapper_init.hpp"
#include "shared/config/config_loader.hpp"
#include "shared/file_operations/file_operations.hpp"
#include "shared/util.hpp"
#include "tickers/manager/ticker_manager.hpp"
#include "traders/trader_container.hpp"
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
    using ticks::TickJobScheduler;

    EngineManager::get_instance().add_engine(ticker, starting_price);

    bots::BotContainer& bot_container =
        EngineManager::get_instance().get_bot_container(ticker);

    // Should run after stale order removal, so they can react to removed orders
    TickJobScheduler::get().on_tick(&bot_container, /*priority=*/2, "Bot Engine");

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
    ticks::TickJobScheduler::get().on_tick(&dashboard, /*priority=*/3, "Dashboard");
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
    ticks::TickJobScheduler::get().on_tick(
        &engine_manager, /*priority=*/1, "Matching Engine"
    );
}

void
initialize_wrappers()
{
    traders::TraderContainer& users = traders::TraderContainer::get_instance();
    rabbitmq::RabbitMQConnectionManager::get_instance().initialize_connection();

    spawning::spawn_all_clients(users);

    rabbitmq::RabbitMQWrapperInitializer::wait_for_clients(users);
    size_t wait_secs = config::Config::get_instance().constants().WAIT_SECS;
    rabbitmq::RabbitMQWrapperInitializer::send_start_time(users, wait_secs);
}

void
start_tick_scheduler()
{
    auto tick_hz = config::Config::get_instance().constants().TICK_HZ;
    ticks::TickJobScheduler::get().start(tick_hz);
}

void
initialize_algos(const auto& mode, const auto& sandbox)
{
    traders::TraderContainer& users = traders::TraderContainer::get_instance();
    auto algo_mgr = algos::AlgoInitializer::get_algo_initializer(mode, sandbox);
    algo_mgr->initialize_algo_management(users);
}

void
blocking_event_loop(const auto& mode)
{
    if (mode == util::Mode::bots_only) {
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

    auto prox_args = config::process_arguments(argc, argv);
    auto mode = std::get<0>(prox_args);
    auto sandbox = std::get<1>(prox_args);

    // Algos must init before wrappers
    initialize_algos(mode, sandbox);

    if (mode != util::Mode::bots_only)
        initialize_wrappers();

    initialize_tickers();
    initialize_bots();
    initialize_dashboard();
    start_tick_scheduler();

    concurrency::pin_to_core(0, "main");

    blocking_event_loop(mode);
    return 0;
}
