#include "algos/algo_manager.hpp"
#include "exchange/algos/algo_manager.hpp"
#include "exchange/config/dynamic/argparse.hpp"
#include "exchange/config/dynamic/config.hpp"
#include "exchange/metrics/on_tick_metrics.hpp"
#include "exchange/tick_scheduler/tick_scheduler.hpp"
#include "logging.hpp"
#include "sandbox/server/crow.hpp"
#include "shared/util.hpp"
#include "tickers/manager/ticker_manager.hpp"
#include "traders/trader_container.hpp"
#include "wrappers/creation/rmq_wrapper_init.hpp"
#include "wrappers/messaging/consumer.hpp"

#include <csignal>

namespace {
using namespace nutc; // NOLINT

void
initialize_bots(std::shared_ptr<engine_manager::EngineManager> manager)
{
    const auto& tickers = config::Config::get().get_tickers();
    for (const auto& ticker : tickers) {
        manager->add_engine(ticker);
    }
}

void
initialize_wrappers()
{
    traders::TraderContainer& users = traders::TraderContainer::get_instance();

    size_t wait_secs = config::Config::get().constants().WAIT_SECS;
    rabbitmq::WrapperInitializer::send_start_time(users.get_traders(), wait_secs);
}

void
initialize_algos(const auto& mode)
{
    traders::TraderContainer& users = traders::TraderContainer::get_instance();
    auto algo_mgr = algos::AlgoInitializer::get_algo_initializer(mode);
    algo_mgr->initialize_algo_management(
        users, config::Config::get().constants().STARTING_CAPITAL
    );
}

auto
create_runner(auto manager)
{
    auto consumer = std::make_shared<rabbitmq::WrapperConsumer>(manager);
    auto metrics = std::make_shared<metrics::OnTickMetricsPush>(manager);

    ticks::TickJobScheduler scheduler;
    scheduler.on_tick(consumer, /*priority=*/1, "consumer");
    scheduler.on_tick(manager, /*priority=*/2, "Matching Engine");
    scheduler.on_tick(metrics, /*priority=*/3, "Metrics Pushing");
    return scheduler;
}
} // namespace

int
main(int argc, const char** argv)
{
    logging::init(quill::LogLevel::Info);

    std::signal(SIGINT, [](auto) { std::exit(0); });
    std::signal(SIGPIPE, SIG_IGN);

    auto mode = config::process_arguments(argc, argv);

    auto engine_manager = std::make_shared<engine_manager::EngineManager>();

    // Algos must init before wrappers
    initialize_algos(mode);

    if (mode != util::Mode::bots_only)
        initialize_wrappers();

    initialize_bots(engine_manager);

    sandbox::CrowServer::get_instance();

    auto tick_hz = config::Config::get().constants().TICK_HZ;
    create_runner(engine_manager).run(tick_hz);
    return 0;
}
