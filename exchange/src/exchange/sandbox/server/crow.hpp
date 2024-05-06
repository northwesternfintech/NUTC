#pragma once

#include "exchange/logging.hpp"
#include "exchange/traders/trader_container.hpp"
#include "exchange/traders/trader_types/algo_trader.hpp"
#include "shared/config/config_loader.hpp"

#include <boost/asio.hpp>
#include <crow/app.h>

#include <iostream>
#include <string>
#include <thread>

namespace nutc {
namespace sandbox {
namespace ba = boost::asio;

class CrowServer {
    ba::io_context io_context_;
    ba::executor_work_guard<ba::io_context::executor_type> work_guard_;
    std::thread timer_thread;

    crow::SimpleApp app{};
    std::thread server_thread;
    std::vector<std::shared_ptr<ba::steady_timer>> timers_{};

public:
    static CrowServer&
    get_instance()
    {
        static CrowServer server{};
        return server;
    }

    CrowServer();

    ~CrowServer();

private:
    void start_remove_timer(unsigned int time_ms, const std::string& trader_id);
};

} // namespace sandbox
} // namespace nutc
