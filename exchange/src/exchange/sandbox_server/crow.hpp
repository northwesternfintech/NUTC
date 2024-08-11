#pragma once

#include "exchange/traders/trader_types/algo_trader.hpp"

#include <boost/asio.hpp>
#include <crow/app.h>

#include <mutex>
#include <queue>
#include <thread>

namespace nutc {
namespace sandbox {
namespace ba = boost::asio;
using lock_guard = std::lock_guard<std::mutex>;

class CrowServer {
    ba::io_context io_context_;
    ba::executor_work_guard<ba::io_context::executor_type> work_guard_;
    std::thread timer_thread;

    crow::SimpleApp app{};
    std::thread server_thread;
    std::vector<ba::steady_timer> timers_{};

    mutable std::mutex trader_lock;
    std::vector<std::shared_ptr<traders::AlgoTrader>> traders_to_add;

public:
    std::vector<std::shared_ptr<traders::AlgoTrader>>
    get_and_clear_pending_traders()
    {
        lock_guard guard{trader_lock};

        auto pending_traders = std::move(traders_to_add);
        traders_to_add.clear();
        return pending_traders;
    }

    static CrowServer&
    get_instance()
    {
        static CrowServer server{};
        return server;
    }

    CrowServer();

    ~CrowServer();

private:
    void add_pending_trader(const std::string user_id, const std::string algo_id);
    void start_remove_timer_(
        unsigned int time_ms, std::weak_ptr<traders::GenericTrader> trader_ptr
    );
};

} // namespace sandbox
} // namespace nutc
