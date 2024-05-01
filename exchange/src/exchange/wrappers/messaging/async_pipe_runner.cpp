#include "async_pipe_runner.hpp"

#include <boost/asio/error.hpp>
#include <boost/asio/read_until.hpp>
#include <fmt/format.h>

#include <ranges>

namespace nutc {
namespace wrappers {

void
AsyncPipeRunner::remove_pipe(std::shared_ptr<bp::async_pipe> pipe)
{
    std::lock_guard<std::mutex> lock{message_lock_};
    pipe->cancel();

    std::erase_if(pipes_, [&pipe](const auto& pipe_ptr) { return pipe_ptr == pipe; });
}

AsyncPipeRunner::~AsyncPipeRunner()
{
    for (const std::shared_ptr<bp::async_pipe>& pipe : pipes_) {
        pipe->cancel();
    }

    if (!ios.stopped())
        ios.stop();

    if (ios_thread.joinable())
        ios_thread.join();
}

std::weak_ptr<bp::async_pipe>
AsyncPipeRunner::create_pipe(bool start_read)
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
AsyncPipeRunner::get_message()
{
    std::lock_guard<std::mutex> lock{message_lock_};
    if (messages_.empty())
        return std::nullopt;

    auto rv = messages_.front();
    messages_.pop();
    return rv;
}

void
AsyncPipeRunner::async_read_pipe(
    std::shared_ptr<bp::async_pipe> pipe_in, std::shared_ptr<std::string> buffer
)
{
    auto prox_message = [this, buffer, pipe_in](const auto& ec, auto length) {
        if (!ec) [[likely]] {
            message_lock_.lock();
            messages_.push(std::string(buffer->data(), length));
            buffer->erase(0, length);
            message_lock_.unlock();
            async_read_pipe(std::move(pipe_in), std::move(buffer));
        }
        else if (ec == boost::asio::error::eof) {
            async_read_pipe(std::move(pipe_in));
        }
        else {
            // TODO: report a better way
            // throw std::runtime_error(
            // fmt::format("Error reading from wrapper pipe: {}", ec.message())
            // );
        }
    };
    ba::async_read_until(*pipe_in, ba::dynamic_buffer(*buffer), "\n", prox_message);
}

} // namespace wrappers
} // namespace nutc
