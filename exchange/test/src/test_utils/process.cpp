#include "process.hpp"

#include "signal.h"

namespace nutc {
namespace testing_utils {

void
kill_all_processes(const manager::ClientManager& users)
{
    for (const auto& [_, client] : users.get_clients()) {
        kill(client.pid, SIGKILL);
    }
}
} // namespace testing_utils
} // namespace nutc
