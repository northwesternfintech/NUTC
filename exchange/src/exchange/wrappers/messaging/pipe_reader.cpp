#include "pipe_reader.hpp"

#include "async_pipe_runner.hpp"
#include "exchange/logging.hpp"

#include <boost/asio.hpp>
#include <glaze/glaze.hpp>

namespace nutc {
namespace wrappers {

namespace ba = boost::asio;

void
PipeReader::async_read_pipe()
{
    async_read_pipe(std::make_shared<std::string>());
}

std::vector<std::variant<messages::init_message, messages::market_order>>
PipeReader::get_messages()
{
    std::lock_guard<std::mutex> lock{message_lock_};
    auto ret = message_queue_;
    message_queue_.clear();
    return ret;
}

PipeReader::~PipeReader()
{
    auto pipe = pipe_in_ptr.lock();
    if (pipe == nullptr)
        return;
    AsyncPipeRunner::get().remove_pipe(pipe);
}

PipeReader::PipeReader() : pipe_in_ptr(AsyncPipeRunner::get().create_pipe())
{
    async_read_pipe();
}

void
PipeReader::store_message_(const std::string& message)
{
    std::variant<init_message, market_order> data;
    auto err = glz::read_json(data, message);

    // TODO: handle better
    if (err)
        log_e(
            main, "Error processing message from wrapper: {}",
            glz::format_error(err, message)
        );

    std::lock_guard<std::mutex> lock{message_lock_};
    message_queue_.push_back(std::move(data));
}

void
PipeReader::async_read_pipe(std::shared_ptr<std::string> buffer)
{
    auto pipe_in = pipe_in_ptr.lock();
    if (pipe_in == nullptr)
        return;
    auto prox_message = [this, buffer](const auto& ec, auto length) {
        if (!ec) [[likely]] {
            auto mess = std::string(buffer->data(), length);
            store_message_(mess);
            buffer->erase(0, length);
            async_read_pipe(std::move(buffer));
        }
        else if (ec == boost::asio::error::eof) {
            async_read_pipe();
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
