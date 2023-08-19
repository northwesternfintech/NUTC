#include "spawning.hpp"

#include "config.h"

namespace nutc {
namespace client {
void
spawn_all_clients()
{
    std::string endpoint = std::string(FIREBASE_URL) + std::string("users.json");
    glz::json_t res = nutc::client::firebase_request("GET", endpoint);
    glz::json_t::object_t users = res.get<glz::json_t::object_t>();

    std::cout << "Starting exchange with " << users.size() << " users" << std::endl;
    for (auto& [uid, user] : users) {
        std::cout << "Spawning client: " << uid << std::endl;
        spawn_client(uid);
    };
}

void
spawn_client(const std::string& uid)
{
    pid_t pid = fork();
    if (pid == 0) {
        char* args[] = {(char*)"NUTC-client", (char*)uid.c_str(), NULL};
        execvp(args[0], args);

        std::cerr << "Failed to execute NUTC-client\n";
        exit(1);
    }
    else if (pid < 0) {
        std::cerr << "Failed to fork\n";
        exit(1);
    }
}
} // namespace client
} // namespace nutc
