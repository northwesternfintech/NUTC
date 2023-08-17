#include "spawning.hpp"

#include "config.h"

namespace nutc {
namespace client {
void
spawn_all_clients()
{
    std::string endpoint = std::string(FIREBASE_URL) + std::string("users.json");
    std::string res = nutc::client::firebase_request("GET", endpoint);
    std::cout << res << std::endl;
};
} // namespace client
} // namespace nutc
