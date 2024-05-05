#include "rmq_wrapper_init.hpp"

#include "exchange/logging.hpp"

#include <iostream>

namespace nutc {
namespace rabbitmq {

void
WrapperInitializer::send_start_time(
    traders::TraderContainer& manager, size_t wait_seconds
)
{
    using time_point = std::chrono::high_resolution_clock::time_point;
    time_point time =
        std::chrono::high_resolution_clock::now() + std::chrono::seconds(wait_seconds);
    int64_t time_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(time)
                          .time_since_epoch()
                          .count();

    messages::start_time message{time_ns};
    std::string buf = glz::write_json(message);

    manager.broadcast_messages({buf});
}

} // namespace rabbitmq
} // namespace nutc
