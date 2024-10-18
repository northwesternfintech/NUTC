#include "pipe_writer.hpp"

#include "async_pipe_runner.hpp"
#include "exchange/config/static/config.hpp"

#include <iostream>

namespace nutc::exchange {

PipeWriter::PipeWriter() :
    pipe_context_(AsyncPipeRunner::get_context()), pipe_out_(*pipe_context_)
{}

PipeWriter::~PipeWriter()
{
    pipe_out_.cancel();
    pipe_out_.close();
}

void
PipeWriter::send_message(const std::string& message)
{
    std::lock_guard<std::mutex> lock{message_lock_};
    queued_shared_.push_back(message);
    if (queued_shared_.size() > MAX_OUTGOING_MQ_SIZE) [[unlikely]] {
        queued_shared_.pop_front();
        // std::cerr << "DROPPED MESSAGE\n";
    }

    // It will enqueue these shared anyway
    if (is_writing_.test_and_set(std::memory_order_acquire)) {
        return;
    }
    auto to_send = queued_shared_.front();
    queued_shared_.pop_front();
    async_write_pipe(to_send + "\n");
}

void
PipeWriter::async_write_pipe(std::string message)
{
    auto buf = std::make_shared<std::string>(std::move(message));
    boost::asio::async_write(
        pipe_out_, bp::buffer(*buf),
        [this, buf](boost::system::error_code ec, std::size_t) {
            if (!ec) [[likely]] {
                std::lock_guard<std::mutex> lock{message_lock_};
                if (queued_shared_.empty()) {
                    is_writing_.clear(std::memory_order_release);
                    return;
                }

                std::string to_write = queued_shared_.front();
                queued_shared_.pop_front();
                async_write_pipe(to_write + "\n");
            }
            else {
                // Ignore the error, but stop writing
                is_writing_.clear(std::memory_order_release);
            }
        }
    );
}
} // namespace nutc::exchange
