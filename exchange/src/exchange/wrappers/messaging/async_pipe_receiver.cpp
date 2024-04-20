#include "async_pipe_receiver.hpp"

#include <boost/asio/error.hpp>
#include <fmt/format.h>

namespace nutc {
namespace wrappers {

AsyncPipeReceiver::~AsyncPipeReceiver()
{
    for (const std::shared_ptr<bp::async_pipe>& pipe : pipes_) {
        pipe->cancel();
    }

    if (!ios.stopped())
        ios.stop();

    if (ios_thread.joinable())
        ios_thread.join();
}

std::shared_ptr<bp::async_pipe>
AsyncPipeReceiver::create_pipe(bool start_read)
{
    std::lock_guard<std::mutex> lock{message_lock_};
    pipes_.emplace_back(std::make_shared<bp::async_pipe>(ios));

    if (start_read) {
        if (!ios_thread.joinable()) [[unlikely]] {
            ios_thread = std::thread([this]() { ios.run(); });
        }

        async_read_pipe(pipes_.back());
    }

    return pipes_.back();
}

std::optional<std::string>
AsyncPipeReceiver::get_message()
{
    std::lock_guard<std::mutex> lock{message_lock_};
    if (messages_.empty())
        return std::nullopt;

    auto rv = messages_.front();
    messages_.pop();
    return rv;
}

void
AsyncPipeReceiver::async_read_pipe(std::shared_ptr<bp::async_pipe> pipe_in)
{
    auto buffer = std::make_shared<std::array<char, 1024>>();
    auto prox_message = [this, buffer, pipe_in](const auto& ec, auto length) {
        if (!ec) [[likely]] {
            message_lock_.lock();
            messages_.push(std::string(buffer->data(), length));
            message_lock_.unlock();
            async_read_pipe(pipe_in);
        }
        else if (ec == boost::asio::error::eof) {
            async_read_pipe(pipe_in);
        }
        // else if (ec == boost::asio::error::operation_aborted) {exit(1);}
        else {
            throw std::runtime_error(
                fmt::format("Error reading from wrapper pipe: {}", ec.message())
            );
        }
    };
    pipe_in->async_read_some(ba::buffer(*buffer), prox_message);
}

} // namespace wrappers
} // namespace nutc
