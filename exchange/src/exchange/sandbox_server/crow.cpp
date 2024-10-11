#include "crow.hpp"

#include "common/fetching/fetching.hpp"
#include "common/logging/logging.hpp"
#include "common/messages_exchange_to_wrapper.hpp"
#include "common/types/algorithm/base_algorithm.hpp"
#include "exchange/config/dynamic/config.hpp"
#include "exchange/config/static/config.hpp"
#include "exchange/traders/trader_types/algo_trader.hpp"

#include <boost/filesystem/operations.hpp>
#include <crow/common.h>
#include <unistd.h>

namespace nutc::exchange {

CrowServer::CrowServer(std::uint16_t port) :
    work_guard_(ba::make_work_guard(io_context_)),
    timer_thread([this]() { io_context_.run(); })
{
    CROW_ROUTE(app, "/sandbox/<string>/<string>")
        .methods(crow::HTTPMethod::POST, crow::HTTPMethod::OPTIONS)(
            [this](
                const crow::request& req, std::string algo_id, std::string language
            ) {
                crow::response res;
                res.code = 200;
                // Set CORS headers
                res.add_header("Access-Control-Allow-Origin", "*");
                res.add_header("Access-Control-Allow-Methods", "POST, OPTIONS");
                res.add_header(
                    "Access-Control-Allow-Headers", "Origin, Content-Type, Accept"
                );

                // Handle preflight request for OPTIONS method
                if (req.method == crow::HTTPMethod::OPTIONS) {
                    res.code = 204;
                    return res;
                }

                if (!req.url_params.get("logfile_url")) {
                    log_e(main, "No logfile_url provided");
                    return crow::response(400);
                }

                std::string logfile_url = req.url_params.get("logfile_url");

                try {
                    log_i(
                        sandbox_server,
                        "Received sandbox request with algo_id {} and language {}. "
                        "Pre-signed url: {}",
                        algo_id, language, logfile_url
                    );
                    common::AlgoLanguage language_enum =
                        language == "Python" ? common::AlgoLanguage::python
                                             : common::AlgoLanguage::cpp;
                    std::string algorithm_data = req.body;
                    add_pending_trader_(
                        algo_id, language_enum, algorithm_data, logfile_url
                    );
                    crow::json::wvalue response_json({
                        {"success", true                              },
                        {"message", "Algorithm Successfully Submitted"}
                    });
                    res.body = response_json.dump();
                    return res;
                } catch (const std::exception& e) {
                    log_e(sandbox_server, "Failed to spawn algorithm: {}", e.what());
                    crow::json::wvalue response_json({
                        {"success", false   },
                        {"message", e.what()}
                    });
                    res.body = response_json.dump();
                    return crow::response(500, e.what());
                }
            }
        );
    server_thread = std::jthread([this, port] { app.signal_clear().port(port).run(); });
}

void
CrowServer::add_pending_trader_(
    const std::string& algo_id, common::AlgoLanguage language,
    const std::string& algorithm_data, const std::string& logfile_url
)
{
    static const auto STARTING_CAPITAL = Config::get().constants().STARTING_CAPITAL;

    auto trader = std::make_shared<AlgoTrader>(
        common::RemoteAlgorithm{language, algo_id, algorithm_data}, STARTING_CAPITAL
    );

    trader_lock.lock();
    traders_to_add.push_back(trader);
    trader_lock.unlock();

    static auto trial_secs = Config::get().constants().SANDBOX_TRIAL_SECS;
    start_remove_timer_(trial_secs, trader, algo_id, logfile_url);

    auto get_start_message = []() {
        static auto start_message = glz::write_json(common::start_time{0});
        if (!start_message.has_value()) [[unlikely]]
            throw std::runtime_error(glz::format_error(start_message.error()));
        return start_message.value();
    };

    static auto start_message = get_start_message();
    trader->send_message(start_message);
}

CrowServer::~CrowServer()
{
    io_context_.stop();
}

void
CrowServer::start_remove_timer_(
    unsigned int time_ms, std::weak_ptr<GenericTrader> trader_ptr,
    const std::string& algo_id, const std::string& logfile_url
)
{
    auto timer = ba::steady_timer{io_context_, std::chrono::seconds(time_ms)};

    timer.async_wait([trader_ptr, logfile_url,
                      algo_id](const boost::system::error_code& err_code) {
        auto trader = trader_ptr.lock();
        if (trader == nullptr) {
            log_i(sandbox_server, "Trader already removed: {}", algo_id);
            return;
        }
        if (err_code) {
            log_e(
                sandbox_server, "Unable to remove trader {}", trader->get_display_name()
            );
        }
        else {
            log_i(sandbox_server, "Removing trader {}", trader->get_display_name());
            trader->disable();
        }

        std::ifstream log_file(fmt::format("{}/{}.log", LOG_DIR, algo_id));
        if (!log_file.is_open()) {
            log_e(
                sandbox_server, "Unable to open log file for trader {}",
                trader->get_display_name()
            );
            return;
        }
        std::stringstream log_ss;
        log_ss << log_file.rdbuf();

        client::put_request(logfile_url, log_ss.str());
    });
    timers_.push_back(std::move(timer));
}

} // namespace nutc::exchange
