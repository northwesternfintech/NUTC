#include "system.hpp"

namespace nutc::wrapper {
namespace {
// We stop the exchange with sigint. The wrapper should exit gracefully
void
catch_sigint(int)
{
    quill::flush();
    // Wait until we're forced to terminate
    while (true) {}
}

void
catch_sigterm(int)
{
    quill::flush();
    std::terminate();
}
} // namespace

void
set_signal_handlers()
{
    if (std::signal(SIGINT, catch_sigint) == nullptr) {
        throw std::runtime_error("Failed to set sigint signal handler");
    }
    if (std::signal(SIGTERM, catch_sigterm) == nullptr) {
        throw std::runtime_error("Failed to set signal handlers");
    }
}
} // namespace nutc::wrapper
