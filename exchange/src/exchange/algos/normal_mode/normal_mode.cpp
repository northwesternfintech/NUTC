#include "normal_mode.hpp"

#include "exchange/config.h"
#include "exchange/curl/curl.hpp"
#include "exchange/traders/trader_types/remote_trader.hpp"
#include "shared/config/config_loader.hpp"

#include <fmt/format.h>

namespace nutc {
namespace algo_mgmt {

void
NormalModeAlgoManager::initialize_client_manager(manager::TraderManager& users)
{
    constexpr std::array<const char*, 3> REQUIRED_FIEDS = {
        "latestAlgoId", "firstName", "lastName"
    };
    int starting_cap = config::Config::get_instance().constants().STARTING_CAPITAL;

    glz::json_t::object_t firebase_users = get_all_users();
    for (const auto& [user_id, user] : firebase_users) {
        bool contains_all_fields = std::all_of(
            REQUIRED_FIEDS.begin(), REQUIRED_FIEDS.end(),
            [&user](const char* field) { return user.contains(field); }
        );
        if (!contains_all_fields) {
            continue;
        }

        std::string full_name = fmt::format(
            "{} {}", user["firstName"].get<std::string>(),
            user["lastName"].get<std::string>()
        );
        std::string algo_id = user["latestAlgoId"].get<std::string>();
        users.add_trader<manager::RemoteTrader>(
            user_id, full_name, algo_id, starting_cap
        );
    }
}

glz::json_t::object_t
NormalModeAlgoManager::get_all_users()
{
    std::string endpoint = std::string(FIREBASE_URL) + std::string("users.json");
    glz::json_t res = curl::request_to_json("GET", endpoint);
    return res.get<glz::json_t::object_t>();
}

} // namespace algo_mgmt
} // namespace nutc
