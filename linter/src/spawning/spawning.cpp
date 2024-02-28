#include "spawning.hpp"

namespace nutc {
namespace spawning {

void
spawn_client(const std::string& uid, std::string& algoid)
{
    std::replace(algoid.begin(), algoid.end(), '-', ' ');
    pid_t pid = fork();

    if (pid == 0) {
        std::vector<std::string> args = {
            "NUTC-linter-spawner", "--uid", uid, "--algoid", algoid
        };

        std::vector<char*> c_args;
        for (auto& arg : args)
            c_args.push_back(arg.data());
        c_args.push_back(nullptr);

        execvp(c_args[0], c_args.data());

        log_e(linting, "Failed to lint algoid {} for uid {}", algoid, uid);

        exit(1);
    }
    else if (pid < 0) {
        log_e(linting, "Failed to fork");
    }
}
} // namespace spawning
} // namespace nutc
