#include "wrapper/config/argparse.hpp"
#include "wrapper/messaging/exchange_communicator.hpp"
#include "wrapper/pywrapper/pywrapper.hpp"
#include "wrapper/resource_limits.hpp"

#include <pybind11/pybind11.h>
#include <sys/prctl.h>

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
    prctl(PR_SET_PDEATHSIG, SIGKILL);
    std::signal(SIGINT, catch_sigint);
    auto [verbosity, uid, algo_id, development_mode] =
        nutc::config::process_arguments(argc, argv);
    pybind11::scoped_interpreter guard{};

    nutc::limits::set_memory_limit(1024);

    nutc::messaging::ExchangeCommunicator exchange_communicator{};
    nutc::messaging::algorithm_content algorithm =
        exchange_communicator.consume_algorithm();
    std::string algorithm_str = algorithm.algorithm_content_str;

    std::string trader_id = nutc::util::trader_id(uid, algo_id);
    if (development_mode) {
        trader_id = algo_id;
    }

    // Send message to exchange to let it know we successfully initialized
    exchange_communicator.publish_init_message();
    if (algorithm_str.empty()) {
        return 1;
    }
    exchange_communicator.wait_for_start_time();

    nutc::pywrapper::create_api_module(
        exchange_communicator.limit_order_func(),
        exchange_communicator.market_order_func()
    );
    nutc::pywrapper::run_code_init(algorithm_str);

    exchange_communicator.main_event_loop(trader_id);
    return 0;
}
