#include "normal_mode.hpp"

#include "curl/curl.hpp"

namespace nutc {
namespace algo_mgmt {

void
NormalModeAlgoManager::initialize_client_manager(manager::ClientManager& users)
{
    num_clients_ = 0;

    glz::json_t::object_t firebase_users = get_all_users();
    for (const auto& [id, user] : firebase_users) {
        if (!user.contains("latestAlgoId"))
            continue;
        users.add_client(
            id, user["latestAlgoId"].get<std::string>(), /*is_local_algo=*/false
        );
        num_clients_ += 1;
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