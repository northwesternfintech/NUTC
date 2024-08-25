#include "normal_mode.hpp"

#include "exchange/curl/curl.hpp"
#include "exchange/logging.hpp"
#include "exchange/traders/trader_types/algo_trader.hpp"
#include "exchange/wrappers/creation/rmq_wrapper_init.hpp"

#include <fmt/format.h>

#include <algorithm>

namespace nutc::exchange {

void
NormalModeAlgoInitializer::initialize_trader_container(
    TraderContainer& traders, shared::decimal_price start_capital
) const
{
    constexpr const std::array<const char*, 4> REQUIRED_DB_FIELDS = {
        "latestAlgoId", "firstName", "lastName", "algos"
    };
    glz::json_t::object_t firebase_users = get_remote_traders();
    for (const auto& user_it : firebase_users) {
        const auto& user_id = user_it.first;
        const auto& user = user_it.second;

        bool contains_all_fields =
            std::ranges::all_of(REQUIRED_DB_FIELDS, [&user](const char* field) {
                return user.contains(field);
            });
        if (!contains_all_fields) {
            continue;
        }

        std::string full_name = fmt::format(
            "{} {}", user["firstName"].get<std::string>(),
            user["lastName"].get<std::string>()
        );
        std::string algo_id = user["latestAlgoId"].get<std::string>();

        try {
            traders.add_trader<AlgoTrader>(user_id, algo_id, full_name, start_capital);
            log_i(main, "Created user");
        } catch (const std::runtime_error& err) {
            log_w(main, "Failed to create user {}", user_id);
        }
    }

    int64_t start_time = get_start_time(WAIT_SECS);
    std::for_each(traders.begin(), traders.end(), [start_time](auto& trader) {
        send_start_time(trader, start_time);
    });
}

glz::json_t::object_t
NormalModeAlgoInitializer::get_remote_traders()
{
    const std::string endpoint = shared::get_firebase_endpoint("users.json");
    glz::json_t res = request_to_json("GET", endpoint);
    return res.get<glz::json_t::object_t>();
}

} // namespace nutc::exchange
