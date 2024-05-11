#include "algos/algo_manager.hpp"
#include "curl/curl.hpp"
#include "exchange/algos/algo_manager.hpp"
#include "exchange/bots/bot_container.hpp"
#include "exchange/config/argparse.hpp"
#include "exchange/metrics/dashboard.hpp"
#include "exchange/metrics/on_tick_metrics.hpp"
#include "exchange/metrics/state/global_metrics.hpp"
#include "exchange/tick_scheduler/tick_scheduler.hpp"
#include "logging.hpp"
#include "sandbox/server/crow.hpp"
#include "shared/config/config_loader.hpp"
#include "shared/file_operations/file_operations.hpp"
#include "shared/util.hpp"
#include "tickers/manager/ticker_manager.hpp"
#include "traders/trader_container.hpp"
#include "wrappers/creation/rmq_wrapper_init.hpp"
#include "wrappers/messaging/consumer.hpp"

#include <csignal>

#include <string>

namespace {
using namespace nutc; // NOLINT

void
flush_log(int)
{
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
    TickJobScheduler::get().on_tick(
        &bot_container, /*priority=*/3, fmt::format("Bot Engine for ticker {}", ticker)
    );

    dashboard::DashboardState::get_instance().add_ticker(ticker, starting_price);
}

void
initialize_tickers()
{
    const auto& tickers = config::Config::get().get_tickers();
    for (const auto& ticker : tickers) {
        initialize_indiv_ticker(ticker.TICKER, ticker.STARTING_PRICE);
    }
}

void
initialize_dashboard()
{
    auto& dashboard = dashboard::Dashboard::get_instance();
    ticks::TickJobScheduler::get().on_tick(&dashboard, /*priority=*/4, "Dashboard");
}

void
initialize_bots()
{
    auto bots = config::Config::get().get_bots();
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
        else if (bot.TYPE == config::BotType::bollinger) {
            container.add_bots<bots::BollingerBot>(
                bot.AVERAGE_CAPITAL, bot.STD_DEV_CAPITAL, bot.NUM_BOTS
            );
        }
    }
    // TODO(stevenewald): should this be somewhere else?
    ticks::TickJobScheduler::get().on_tick(
        &engine_manager, /*priority=*/2, "Matching Engine"
    );
}

void
initialize_wrappers()
{
    traders::TraderContainer& users = traders::TraderContainer::get_instance();

    size_t wait_secs = config::Config::get().constants().WAIT_SECS;
    rabbitmq::WrapperInitializer::send_start_time(users, wait_secs);
}

void
start_tick_scheduler()
{
    auto tick_hz = config::Config::get().constants().TICK_HZ;
    ticks::TickJobScheduler::get().start(tick_hz);
}

void
initialize_algos(const auto& mode)
{
    traders::TraderContainer& users = traders::TraderContainer::get_instance();
    auto algo_mgr = algos::AlgoInitializer::get_algo_initializer(mode);
    algo_mgr->initialize_algo_management(users);
}

void
on_tick_consumer()
{
    static rabbitmq::WrapperConsumer consumer{};
    ticks::TickJobScheduler::get().on_tick(&consumer, /*priority=*/8, "consumer");
}
} // namespace

int
main(int argc, const char** argv)
{
    using namespace nutc; // NOLINT(*)
    logging::init(quill::LogLevel::Info);

    std::signal(SIGINT, flush_log);
    std::signal(SIGPIPE, SIG_IGN);

    auto mode = config::process_arguments(argc, argv);

    // Algos must init before wrappers
    initialize_algos(mode);

    if (mode != util::Mode::bots_only)
        initialize_wrappers();

    initialize_tickers();
    initialize_bots();
    initialize_dashboard();
    on_tick_consumer();

    ticks::TickJobScheduler::get().on_tick(
        &(metrics::OnTickMetricsPush::get()), /*priority=*/5, "Metrics Pushing"
    );

    start_tick_scheduler();
    return 0;
}
