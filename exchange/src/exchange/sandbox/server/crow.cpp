#include "crow.hpp"

#include "exchange/curl/curl.hpp"
#include "exchange/logging.hpp"
#include "exchange/traders/trader_container.hpp"
#include "exchange/traders/trader_types/algo_trader.hpp"
#include "shared/config/config_loader.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"

namespace nutc {
namespace sandbox {

CrowServer::CrowServer() :
    work_guard_(ba::make_work_guard(io_context_)),
    timer_thread([this]() { io_context_.run(); })
{
    CROW_ROUTE(app, "/sandbox/<string>/<string>")
    ([this](const crow::request& req, std::string user_id, std::string algo_id) {
        crow::response res;
        res.code = 200;
        // Set CORS headers
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header(
            "Access-Control-Allow-Methods", "GET, POST, PATCH, PUT, DELETE, OPTIONS"
        );
        res.add_header("Access-Control-Allow-Headers", "Origin, Content-Type, Accept");

        // Handle preflight request for OPTIONS method
        if (req.method == crow::HTTPMethod::OPTIONS) {
            res.code = 204;
            return res;
        }

        static auto trial_secs = config::Config::get().constants().SANDBOX_TRIAL_SECS;
        try {
            log_i(
                main, "Received sandbox request with user id {} and algoid {}", user_id,
                algo_id
            );
            static const auto STARTING_CAPITAL =
                nutc::config::Config::get().constants().STARTING_CAPITAL;
            auto trader = traders::TraderContainer::get_instance()
                              .add_trader<traders::LocalTrader>(
                                  user_id, algo_id, "SANDBOX_USER", STARTING_CAPITAL
                              );
            start_remove_timer_(
                trial_secs, trader,
                fmt::format("users/{}/algos/{}/sandbox_results.json", user_id, algo_id)
            );
            trader->send_messages({glz::write_json(messages::start_time{0})});
            return res;
        } catch (...) {
            return crow::response(500);
        }
    });
    server_thread = std::thread([this] { app.signal_clear().port(18080).run(); });
}

CrowServer::~CrowServer()
{
    app.stop();

    io_context_.stop();

    if (server_thread.joinable())
        server_thread.join();

    if (timer_thread.joinable())
        timer_thread.join();
}

void
CrowServer::start_remove_timer_(
    unsigned int time_s, const std::weak_ptr<traders::GenericTrader>& trader_ptr,
    const std::string& sandbox_log_path
)
{
    auto timer = ba::steady_timer{io_context_, std::chrono::seconds(time_s)};

    timer.async_wait([trader_ptr,
                      sandbox_log_path](const boost::system::error_code& err_code) {
        auto trader = trader_ptr.lock();
        if (trader == nullptr) {
            log_i(main, "Trader already removed: {}", trader->get_display_name());
            return;
        }
        std::string trader_id = trader->get_id();
        if (!err_code) {
            log_i(main, "Removing trader {}", trader->get_display_name());
            traders::TraderContainer::get_instance().remove_trader(trader);
        }
        else {
            log_e(main, "Unable to remove trader {}", trader->get_display_name());
        }

        std::string loc = curl::upload_file(fmt::format("logs/{}", trader_id));

	std::string res = curl::request_to_string(
            "PUT", util::get_firebase_endpoint(sandbox_log_path), loc
        );
	log_i(main, "Result of upload: {} {}", loc, res);
    });
    timers_.push_back(std::move(timer));
}

} // namespace sandbox
} // namespace nutc
