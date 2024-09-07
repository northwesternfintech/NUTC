#include "pipe_reader.hpp"

#include "async_pipe_runner.hpp"
#include "common/messages_wrapper_to_exchange.hpp"
#include "exchange/logging.hpp"

#include <boost/asio.hpp>
#include <glaze/glaze.hpp>

namespace nutc::exchange {

void
PipeReader::async_read_pipe()
{
    async_read_pipe(std::make_shared<std::string>());
}

PipeReader::~PipeReader()
{
    pipe_in_.cancel();
    pipe_in_.close();
}

PipeReader::PipeReader() :
    pipe_context_(AsyncPipeRunner::get_context()), pipe_in_(*pipe_context_)
{
    async_read_pipe();
}

void
PipeReader::store_message_(const std::string& message)
{
    common::IncomingMessageVariant data;
    auto err = glz::read_json(data, message);

    // TODO: handle better
    if (err) [[unlikely]] {
        log_e(
            pipe_reader, "Error processing message from wrapper: {}",
            glz::format_error(err, message)
        );
        return;
    }

    std::lock_guard<std::mutex> lock{message_lock_};
    shared.push_back(data);
}

void
PipeReader::async_read_pipe(std::shared_ptr<std::string> buffer)
{
    auto prox_message = [this, buffer](const auto& ec, auto length) mutable {
        if (!ec) [[likely]] {
            auto mess = std::string(buffer->data(), length);
            store_message_(mess);
            buffer->erase(0, length);
            async_read_pipe(std::move(buffer));
        }
        else {
            // TODO: report a better way
            // throw std::runtime_error(
            //     fmt::format("Error reading from wrapper pipe: {}", ec.message())
            // );
        }
    };
    ba::async_read_until(pipe_in_, ba::dynamic_buffer(*buffer), "\n", prox_message);
}

} // namespace nutc::exchange
