#include "wrapper/config/argparse.hpp"
#include "wrapper/messaging/exchange_communicator.hpp"
#include "wrapper/pywrapper/pywrapper.hpp"
#include "wrapper/util/logging.hpp"
#include "wrapper/util/resource_limits.hpp"

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

#include <csignal>

#include <string>

// We stop the exchange with sigint. The wrapper should exit gracefully
void
catch_sigint(int)
{
    // Wait until we're forced to terminate
    while (true) {}
}

int
main(int argc, const char** argv)
{
    using namespace nutc::messaging;
    using namespace nutc::pywrapper;
    using namespace nutc::system;

    std::signal(SIGINT, catch_sigint);
    auto [verbosity, trader_id] = nutc::config::process_arguments(argc, argv);
    pybind11::scoped_interpreter guard{};

    ExchangeCommunicator communicator{
        trader_id, ob_update_function(), trade_update_function(),
        account_update_function()
    };

    if (!set_memory_limit(1024) || !kill_on_exchange_death()) {
        log_e(main, "Failed to set memory limit");
        communicator.report_startup_complete();
        return 1;
    }

    algorithm_content algorithm = communicator.consume_algorithm();

    if (algorithm.algorithm_content_str.empty()) {
        communicator.report_startup_complete();
        return 1;
    }
    communicator.report_startup_complete();
    communicator.wait_for_start_time();

    nutc::pywrapper::create_api_module(
        communicator.place_limit_order(), communicator.place_market_order()
    );
    nutc::pywrapper::run_initialization_code(algorithm.algorithm_content_str);

    communicator.main_event_loop();
    return 0;
}
