#include "rmq_wrapper_init.hpp"

#include "shared/messages_exchange_to_wrapper.hpp"

namespace nutc {
namespace rabbitmq {

void
WrapperInitializer::send_start_time(
    const std::pmr::vector<TraderPtr>& traders, size_t wait_seconds
)
{
    using time_point = std::chrono::high_resolution_clock::time_point;
    time_point time =
        std::chrono::high_resolution_clock::now() + std::chrono::seconds(wait_seconds);
    int64_t time_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(time)
                          .time_since_epoch()
                          .count();

    messages::start_time message{time_ns};
    auto buf = glz::write_json(message);

    for (const auto& trader : traders) {
        trader->send_message(buf);
    }
}

} // namespace rabbitmq
} // namespace nutc
