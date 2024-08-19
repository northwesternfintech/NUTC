#include "rmq_wrapper_init.hpp"

#include "shared/messages_exchange_to_wrapper.hpp"

namespace nutc {
namespace rabbitmq {

int64_t
get_start_time(size_t wait_seconds)
{
    using hrq = std::chrono::high_resolution_clock;
    hrq::time_point time = hrq::now() + std::chrono::seconds(wait_seconds);
    return std::chrono::time_point_cast<std::chrono::nanoseconds>(time)
        .time_since_epoch()
        .count();
}

void
send_start_time(traders::GenericTrader& trader, int64_t start_time)
{
    trader.send_message(glz::write_json(messages::start_time{start_time}));
}

} // namespace rabbitmq
} // namespace nutc
