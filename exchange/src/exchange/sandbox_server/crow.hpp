#pragma once

#include "exchange/traders/trader_types/algo_trader.hpp"

#include <boost/asio.hpp>
#include <crow/app.h>

#include <mutex>
#include <thread>

namespace nutc::exchange {

namespace ba = boost::asio;
using lock_guard = std::lock_guard<std::mutex>;

class CrowServer {
    ba::io_context io_context_;
    ba::executor_work_guard<ba::io_context::executor_type> work_guard_;
    std::jthread timer_thread;

    crow::SimpleApp app{};
    std::jthread server_thread;
    std::vector<ba::steady_timer> timers_{};

    mutable std::mutex trader_lock;
    std::vector<std::shared_ptr<AlgoTrader>> traders_to_add;

public:
    std::vector<std::shared_ptr<AlgoTrader>>
    get_and_clear_pending_traders()
    {
        lock_guard guard{trader_lock};

        auto pending_traders = std::move(traders_to_add);
        traders_to_add.clear();
        return pending_traders;
    }

    explicit CrowServer(std::uint16_t port);

    ~CrowServer();

private:
    void add_pending_trader_(
        const std::string& algo_id, common::AlgoLanguage language,
        const std::string& algorithm_data, const std::string& logfile_url
    );
    void start_remove_timer_(
        unsigned int time_ms, std::weak_ptr<GenericTrader> trader_ptr,
        const std::string& algo_id, const std::string& logfile_url
    );
};

} // namespace nutc::exchange
