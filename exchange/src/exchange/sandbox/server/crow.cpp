#include "crow.hpp"

#include "shared/messages_exchange_to_wrapper.hpp"

namespace nutc {
namespace sandbox {

CrowServer::CrowServer() : work_guard_(io_context_.get_executor())
{
    CROW_ROUTE(app, "/sandbox/<string>/<string>/<string>")
    ([this](
         const crow::request& req, std::string user_id, std::string algo_id,
         std::string display_name
     ) {
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
                                  user_id, algo_id, display_name, STARTING_CAPITAL
                              );
            trader->send_messages({glz::write_json(messages::start_time{0})});
            start_remove_timer(trial_secs, trader->get_id());
            return res;
        } catch (...) {
            return crow::response(500);
        }
    });

    server_thread = std::thread([this] { app.signal_clear().port(18080).run(); });
    timer_thread = std::thread([this] { io_context_.run(); });
}

CrowServer::~CrowServer()
{
    app.stop();

    for (const auto& timer : timers_)
        timer->cancel();

    io_context_.stop();

    if (server_thread.joinable())
        server_thread.join();

    if (timer_thread.joinable())
        timer_thread.join();
}

void
CrowServer::start_remove_timer(unsigned int time_s, const std::string& trader_id)
{
    auto timer =
        std::make_shared<ba::steady_timer>(io_context_, ba::chrono::seconds(time_s));
    timer->async_wait([timer, trader_id](const boost::system::error_code& ec) {
        if (!ec) {
            log_i(main, "Removing trader {}", trader_id);
            traders::TraderContainer::get_instance().remove_trader(trader_id);
        }
        else {
            log_e(main, "Unable to remove trader {}", trader_id);
        }
    });
    timers_.push_back(timer);
}

} // namespace sandbox
} // namespace nutc
