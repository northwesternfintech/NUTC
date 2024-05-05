#include "pipe_writer.hpp"

#include "async_pipe_runner.hpp"
#include "exchange/config.h"

namespace nutc {
namespace wrappers {

PipeWriter::PipeWriter() : pipe_out_ptr(AsyncPipeRunner::get().create_pipe()) {}

PipeWriter::~PipeWriter()
{
    auto pipe = pipe_out_ptr.lock();
    if (pipe == nullptr)
        return;
    AsyncPipeRunner::get().remove_pipe(pipe);
}

void
PipeWriter::send_messages(std::vector<std::string> messages)
{
    std::lock_guard<std::mutex> lock{message_lock_};
    std::ranges::move(messages, std::back_inserter(queued_messages_));
    while (queued_messages_.size() > MAX_OUTGOING_MQ_SIZE) [[unlikely]] {
        queued_messages_.pop_front();
    }

    // It will enqueue these messages anyway
    if (is_writing_.test_and_set(std::memory_order_acquire)) {
        return;
    }
    auto to_send = queued_messages_.front();
    queued_messages_.pop_front();
    async_write_pipe(to_send + "\n");
}

void
PipeWriter::async_write_pipe(std::string message)
{
    auto buf = std::make_shared<std::string>(std::move(message));
    auto pipe_out = pipe_out_ptr.lock();
    if (pipe_out == nullptr)
        return;
    boost::asio::async_write(
        *pipe_out, bp::buffer(*buf),
        [this, buf](boost::system::error_code ec, std::size_t) {
            if (!ec) [[likely]] {
                std::lock_guard<std::mutex> lock{message_lock_};
                if (queued_messages_.empty()) {
                    is_writing_.clear(std::memory_order_release);
                    return;
                }

                std::string to_write = queued_messages_.front();
                queued_messages_.pop_front();
                async_write_pipe(to_write + "\n");
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
