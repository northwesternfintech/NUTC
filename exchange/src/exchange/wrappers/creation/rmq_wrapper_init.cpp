#include "rmq_wrapper_init.hpp"

#include "exchange/logging.hpp"

#include <iostream>

namespace nutc {
namespace rabbitmq {

void
WrapperInitializer::wait_for_clients(traders::TraderContainer& manager)
{
    size_t num_clients = manager.num_traders();
    log_i(rabbitmq, "Blocking until all {} clients are ready to start...", num_clients);
    int num_running = 0;

    auto is_init = [&](const auto& message) {
        using t = std::decay_t<decltype(message)>;
        if constexpr (std::is_same_v<t, messages::init_message>) {
            log_i(
                rabbitmq, "Received init message with status {}",
                message.ready ? "ready" : "not ready"
            );

            // TODO: maybe send some warning?
            if (!message.ready) {
                // TODO
                //  manager.remove_trader(message.trader_id);
            }
            else {
                num_running++;
            }
            return true;
        }
        return false;
    };

    for (const auto& [id, trader] : manager.get_traders()) {
        if (!trader->record_metrics())
            continue;
        while (true) {
            auto messages = trader->read_messages();
            bool should_break = false;
            for (const auto& mess : messages) {
                if (std::visit(is_init, mess)) {
                    should_break = true;
                }
            }
            if (should_break)
                break;
        }
    }

    log_i(
        rabbitmq, "All {} clients initialized. Starting exchange with {} ready clients",
        num_clients, num_running
    );
}

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
