#include "rmq_wrapper_init.hpp"

#include "common/types/messages/messages_exchange_to_wrapper.hpp"

#include <glaze/json/write.hpp>

namespace nutc::exchange {

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
send_start_time(GenericTrader& trader, int64_t start_time)
{
    static auto mess = glz::write_json(common::start_time{start_time});
    if (!mess.has_value()) [[unlikely]]
        throw std::runtime_error(glz::format_error(mess.error()));

    trader.send_message(*mess);
}

} // namespace nutc::exchange
