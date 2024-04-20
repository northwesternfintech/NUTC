#include "wrapper_handle.hpp"

#include "exchange/config.h"
#include "exchange/wrappers/messaging/async_pipe_receiver.hpp"

#include <boost/asio.hpp>
#include <fmt/format.h>

namespace nutc {
namespace wrappers {
void
WrapperHandle::start(const std::string& path, const std::vector<std::string>& args)
{
    // While we receive a shared_ptr to the pipe, we dereference it to get the actual
    // pipe. This is risky, as we need to make sure we close the pipe before we shut
    // down the wrapper
    pipe_in = AsyncPipeReceiver::get().create_pipe(true);
    pipe_out = AsyncPipeReceiver::get().create_pipe(false);
    wrapper_ = bp::child(
        bp::exe(path), bp::args(args), bp::std_in<*pipe_out, bp::std_err> bp::null,
        bp::std_out > *pipe_in
    );
}

void
WrapperHandle::send_messages(std::vector<std::string> messages)
{
    if (!wrapper_.running() || !pipe_out->is_open()) [[unlikely]]
        return;
    std::lock_guard<std::mutex> lock{messages_lock_};
    if (queued_messages_.size() > MAX_OUTGOING_MQ_SIZE) [[unlikely]] {
        return;
    }
    std::ranges::move(messages, std::back_inserter(queued_messages_));

    // It will enqueue these messages anyway
    if (is_writing_.test_and_set(std::memory_order_acquire)) {
        return;
    }
    auto to_send = queued_messages_.front();
    queued_messages_.pop_front();
    write_out(to_send + "\n");
}

void
WrapperHandle::write_out(std::string message)
{
    auto buf = std::make_shared<std::string>(std::move(message));
    // Pass in buf to keep lifetime
    boost::asio::async_write(
        *pipe_out, bp::buffer(*buf),
        [this, buf](boost::system::error_code ec, std::size_t) {
            if (!ec) [[likely]] {
                std::lock_guard<std::mutex> lock{messages_lock_};
                if (queued_messages_.empty()) {
                    is_writing_.clear(std::memory_order_release);
                    return;
                }

                std::string to_write = queued_messages_.front();
                queued_messages_.pop_front();
                write_out(to_write + "\n");
            }
            else {
                // Ignore the error, but stop writing
                is_writing_.clear(std::memory_order_release);
            }
        }
    );
}

} // namespace wrappers
} // namespace nutc
