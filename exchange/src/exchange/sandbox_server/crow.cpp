#include "crow.hpp"

#include "common/messages_exchange_to_wrapper.hpp"
#include "common/types/algorithm/base_algorithm.hpp"
#include "exchange/config/dynamic/config.hpp"
#include "exchange/logging.hpp"
#include "exchange/traders/trader_types/algo_trader.hpp"

#include <crow/common.h>

namespace nutc::exchange {

CrowServer::CrowServer() :
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

                try {
                    log_i(
                        sandbox_server,
                        "Received sandbox request with algo_id {} and language {}",
                        algo_id, language
                    );
                    common::AlgoLanguage language_enum =
                        language == "Python" ? common::AlgoLanguage::python
                                             : common::AlgoLanguage::cpp;
                    std::string algorithm_data = req.body;
                    add_pending_trader_(algo_id, language_enum, algorithm_data);
                    return res;
                } catch (...) {
                    log_e(sandbox_server, "Failed to spawn algorithm");
                    return crow::response(500);
                }
            }
        );
    server_thread = std::thread([this] { app.signal_clear().port(18080).run(); });
}

void
CrowServer::add_pending_trader_(
    const std::string& algo_id, common::AlgoLanguage language,
    const std::string& algorithm_data
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
    start_remove_timer_(trial_secs, trader);

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
    // Quill deconstruction can cause segfault
    app.loglevel(crow::LogLevel::CRITICAL);
    app.stop();

    io_context_.stop();

    if (server_thread.joinable())
        server_thread.join();

    if (timer_thread.joinable())
        timer_thread.join();
}

void
CrowServer::start_remove_timer_(
    unsigned int time_s, std::weak_ptr<GenericTrader> trader_ptr
)
{
    auto timer = ba::steady_timer{io_context_, std::chrono::seconds(time_s)};

    timer.async_wait([trader_ptr](const boost::system::error_code& err_code) {
        auto trader = trader_ptr.lock();
        if (trader == nullptr) {
            log_i(
                sandbox_server, "Trader already removed: {}", trader->get_display_name()
            );
            return;
        }
        if (!err_code) {
            log_i(sandbox_server, "Removing trader {}", trader->get_display_name());
            trader->disable();
        }
        else {
            log_e(
                sandbox_server, "Unable to remove trader {}", trader->get_display_name()
            );
        }
    });
    timers_.push_back(std::move(timer));
}

} // namespace nutc::exchange
