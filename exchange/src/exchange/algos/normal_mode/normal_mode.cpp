#include "normal_mode.hpp"

#include "common/logging/logging.hpp"
#include "common/util.hpp"
#include "exchange/curl/curl.hpp"
#include "exchange/traders/trader_types/algo_trader.hpp"
#include "exchange/wrappers/creation/rmq_wrapper_init.hpp"

#include <fmt/format.h>

#include <algorithm>

namespace nutc::exchange {

void
NormalModeAlgoInitializer::initialize_trader_container(
    TraderContainer& traders, common::decimal_price start_capital
) const
{
    static int id = 0;
    auto firebase_users = get_remote_traders();
    for (const auto& user : firebase_users) {
        try {
            if (!user.contains("language") || !user.contains("code")
                || !user.contains("name")) {
                throw std::runtime_error("Not contain field");
            }
            if (!user["language"].is_string() || !user["code"].is_string()
                || !user["name"].is_string()) {
                throw std::runtime_error("not string");
            }
            common::AlgoLanguage language =
                user["language"].get<std::string>() == "Python"
                    ? common::AlgoLanguage::python
                    : common::AlgoLanguage::cpp;
            std::string code = user["code"].get<std::string>();
            std::string name = user["name"].get<std::string>();
            traders.add_trader<AlgoTrader>(
                common::RemoteAlgorithm(language, std::to_string(id++), code, name),
                start_capital
            );
            log_i(main, "Created user {}", name);
        } catch (const std::runtime_error& err) {
            log_w(main, "Failed to create user: {}", err.what());
        }
    }

    int64_t start_time = get_start_time(WAIT_SECS);
    std::for_each(traders.begin(), traders.end(), [start_time](auto& trader) {
        send_start_time(trader, start_time);
    });
    std::this_thread::sleep_for(std::chrono::seconds(WAIT_SECS));
}

glz::json_t::array_t
NormalModeAlgoInitializer::get_remote_traders()
{
    const auto* endpoint_c = std::getenv("NUTC_ALGO_ENDPOINT");
    const std::string endpoint{endpoint_c};
    glz::json_t res = request_to_json("GET", endpoint);
    return res.get<glz::json_t::array_t>();
}

} // namespace nutc::exchange
