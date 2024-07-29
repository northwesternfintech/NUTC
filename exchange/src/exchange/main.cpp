#include "algos/algo_manager.hpp"
#include "exchange/algos/algo_manager.hpp"
#include "exchange/config/dynamic/argparse.hpp"
#include "exchange/config/dynamic/config.hpp"
#include "exchange/config/dynamic/ticker_config.hpp"
#include "exchange/logging.hpp"
#include "exchange/matching_cycle/base/base_strategy.hpp"
#include "exchange/matching_cycle/cycle_strategy.hpp"
#include "exchange/matching_cycle/dev/dev_strategy.hpp"
#include "exchange/matching_cycle/sandbox/sandbox_cycle.hpp"
#include "exchange/orders/ticker_info.hpp"
#include "exchange/sandbox_server/crow.hpp"
#include "exchange/traders/trader_container.hpp"
#include "shared/util.hpp"

#include <csignal>

namespace {
using namespace nutc; // NOLINT

matching::TickerMapping
load_tickers()
{
    matching::TickerMapping ret;
    const auto& tickers = config::Config::get().get_tickers();
    for (const config::ticker_config& ticker : tickers) {
        ret.emplace(ticker.TICKER, ticker);
    }
    return ret;
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

std::unique_ptr<matching::MatchingCycle>
create_cycle(const auto& mode)
{
    auto tickers = load_tickers();
    // TODO: not singleton
    auto& traders = traders::TraderContainer::get_instance().get_traders();
    auto exp = config::Config::get().constants().ORDER_EXPIRATION_TICKS;

    if (mode == util::Mode::normal) {
        return std::make_unique<matching::BaseMatchingCycle>(tickers, traders, exp);
    }
    else if (mode == util::Mode::sandbox) {
        return std::make_unique<matching::SandboxMatchingCycle>(tickers, traders, exp);
    }
    else {
        return std::make_unique<matching::DevMatchingCycle>(tickers, traders, exp);
    }
}

void
main_event_loop(auto cycle)
{
    uint64_t tick = 0;
    while (true) {
        cycle->on_tick(tick++);
    }
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

    main_event_loop(create_cycle(mode));

    return 0;
}
