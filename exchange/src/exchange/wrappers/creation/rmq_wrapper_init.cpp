#include "rmq_wrapper_init.hpp"

#include "common/types/messages/messages_exchange_to_wrapper.hpp"

#include <glaze/json/write.hpp>

#include <chrono>

namespace nutc::exchange {

std::chrono::high_resolution_clock::time_point
get_start_time(size_t wait_seconds)
{
    using hrq = std::chrono::high_resolution_clock;
    return hrq::now() + std::chrono::seconds(wait_seconds);
}

void
send_start_time(
    GenericTrader& trader, std::chrono::high_resolution_clock::time_point start_time
)
{
    static auto mess = glz::write_json(common::start_time{start_time});
    if (!mess.has_value()) [[unlikely]]
        throw std::runtime_error(glz::format_error(mess.error()));

    trader.send_message(*mess);
}

} // namespace nutc::exchange
