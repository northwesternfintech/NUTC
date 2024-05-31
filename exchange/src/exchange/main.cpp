#include "algos/algo_manager.hpp"
#include "exchange/algos/algo_manager.hpp"
#include "exchange/config/dynamic/argparse.hpp"
#include "exchange/config/dynamic/config.hpp"
#include "exchange/config/dynamic/ticker_config.hpp"
#include "exchange/tick_scheduler/tick_scheduler.hpp"
#include "exchange/tickers/matching_cycle/dev/dev_strategy.hpp"
#include "exchange/tickers/ticker.hpp"
#include "logging.hpp"
#include "sandbox/server/crow.hpp"
#include "shared/util.hpp"
#include "traders/trader_container.hpp"
#include "wrappers/creation/rmq_wrapper_init.hpp"

#include <csignal>

namespace {
using namespace nutc; // NOLINT

std::unordered_map<std::string, matching::ticker_info>
initialize_bots()
{
    std::unordered_map<std::string, matching::ticker_info> ret;
    const auto& tickers = config::Config::get().get_tickers();
    for (const config::ticker_config& ticker : tickers) {
        ret.emplace(ticker.TICKER, ticker);
    }
    return ret;
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

} // namespace

int
main(int argc, const char** argv)
{
    logging::init(quill::LogLevel::Info);

    std::signal(SIGINT, [](auto) { std::exit(0); });

    // Wrappers may unexpectedly exit for many reasons. Should not affect the exchange
    std::signal(SIGPIPE, SIG_IGN);

    auto mode = config::process_arguments(argc, argv);

    // Algos must init before wrappers
    initialize_algos(mode);

    if (mode != util::Mode::bots_only)
        initialize_wrappers();

    auto tickers = initialize_bots();
    auto cycle = std::make_shared<matching::DevMatchingCycle>(
        tickers, traders::TraderContainer::get_instance().get_traders(),
        config::Config::get().constants().ORDER_EXPIRATION_TICKS
    );

    sandbox::CrowServer::get_instance();

    auto tick_hz = config::Config::get().constants().TICK_HZ;
    nutc::ticks::run([&cycle](uint64_t tick) { cycle->on_tick(tick); }, tick_hz);
    return 0;
}
