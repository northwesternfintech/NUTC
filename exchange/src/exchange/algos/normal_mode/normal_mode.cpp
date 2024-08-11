#include "normal_mode.hpp"

#include "exchange/curl/curl.hpp"
#include "exchange/traders/trader_types/algo_trader.hpp"
#include "exchange/wrappers/creation/rmq_wrapper_init.hpp"
#include "exchange/logging.hpp"

#include <fmt/format.h>

#include <algorithm>

namespace nutc {
namespace algos {

void
NormalModeAlgoInitializer::initialize_trader_container(
    traders::TraderContainer& traders, double start_capital
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
            traders.add_trader<traders::AlgoTrader>(
                user_id, algo_id, full_name, start_capital
            );
			log_i(main, "Created user");
        } catch (const std::runtime_error& err) {
			log_w(main, "Failed to create user {}", user_id);
		}
    }

    rabbitmq::WrapperInitializer::send_start_time(traders.get_traders(), WAIT_SECS);
}

glz::json_t::object_t
NormalModeAlgoInitializer::get_remote_traders()
{
    const std::string endpoint = util::get_firebase_endpoint("users.json");
    glz::json_t res = curl::request_to_json("GET", endpoint);
    return res.get<glz::json_t::object_t>();
}

} // namespace algos
} // namespace nutc
