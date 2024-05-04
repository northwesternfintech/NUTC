#include "pipe_reader.hpp"

#include <boost/asio.hpp>
#include <glaze/glaze.hpp>

namespace nutc {
namespace wrappers {

namespace ba = boost::asio;

void
PipeReader::add_message_(const std::string& message)
{
    std::variant<init_message, market_order> data;
    auto err = glz::read_json(data, message);

    // TODO: handle better
    if (err)
        throw std::runtime_error("idk");

    std::lock_guard<std::mutex> lock{message_lock_};
    message_queue_.push_back(std::move(data));
}

void
PipeReader::async_read_pipe(
    std::weak_ptr<bp::async_pipe> pipe_in_ptr, std::shared_ptr<std::string> buffer
)
{
    auto pipe_in = pipe_in_ptr.lock();
    if (pipe_in == nullptr)
        return;
    auto prox_message = [this, buffer, pipe_in_ptr](const auto& ec, auto length) {
        if (!ec) [[likely]] {
            auto mess = std::string(buffer->data(), length);
            add_message_(mess);
            buffer->erase(0, length);
            async_read_pipe(pipe_in_ptr, std::move(buffer));
        }
        else if (ec == boost::asio::error::eof) {
            async_read_pipe(pipe_in_ptr);
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
