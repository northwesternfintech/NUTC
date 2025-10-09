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
    std::thread timer_thread;

    crow::SimpleApp app{};
    std::thread server_thread;
    std::vector<ba::steady_timer> timers_{};

    mutable std::mutex trader_lock;
    std::vector<std::shared_ptr<AlgoTrader>> traders_to_add;

    std::mutex log_ws_mutex;
    std::unordered_map<std::string, crow::websocket::connection*>
        log_connection_by_algo_id;
    std::unordered_map<crow::websocket::connection*, std::string>
        algo_id_by_log_connection;
    std::vector<std::string> unmapped_algo_ids;

public:
    std::vector<std::shared_ptr<AlgoTrader>>
    get_and_clear_pending_traders()
    {
        lock_guard guard{trader_lock};

        auto pending_traders = std::move(traders_to_add);
        traders_to_add.clear();
        return pending_traders;
    }

    bool is_receiving_logs(const std::string& algo_id);

    void send_log(const std::string& algo_id, const std::string& log_message);

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
