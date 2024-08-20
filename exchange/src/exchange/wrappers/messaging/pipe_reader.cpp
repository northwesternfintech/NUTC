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

auto
PipeReader::get_message() -> ReadMessageVariant
{
    while (true) {
        std::lock_guard<std::mutex> lock{message_lock_};
        if (message_queue_.empty())
            continue;

        auto val = message_queue_.back();
        message_queue_.pop_back();
        return val;
    }
}

auto
PipeReader::get_messages() -> std::vector<ReadMessageVariant>
{
    std::lock_guard<std::mutex> lock{message_lock_};
    auto ret = message_queue_;
    message_queue_.clear();
    return ret;
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
    ReadMessageVariant data;
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
    message_queue_.push_back(std::move(data));
}

void
PipeReader::async_read_pipe(std::shared_ptr<std::string> buffer)
{
    auto prox_message = [this, buffer](const auto& ec, auto length) {
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

} // namespace wrappers
} // namespace nutc
