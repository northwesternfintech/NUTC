#pragma once

#include "exchange/traders/trader_types/trader_interface.hpp"

#include <boost/asio.hpp>
#include <crow/app.h>

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
    std::vector<ba::steady_timer> timers_{};

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
    void start_remove_timer_(
        unsigned int time_s, const std::weak_ptr<traders::GenericTrader>& trader_ptr, const std::string& sandbox_log_path
    );
};

} // namespace sandbox
} // namespace nutc
