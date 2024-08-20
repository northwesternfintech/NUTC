#pragma once
#include "shared/messages_wrapper_to_exchange.hpp"

#include <boost/process.hpp>

#include <mutex>

namespace nutc {
namespace wrappers {

using namespace messages;

namespace bp = boost::process;
namespace ba = boost::asio;

class PipeReader {
public:
    using IncomingMessageVariant =
        std::variant<timed_init_message, timed_limit_order, timed_market_order>;

private:
    std::mutex message_lock_;
    std::vector<IncomingMessageVariant> messages;
    std::shared_ptr<ba::io_context> pipe_context_;
    bp::async_pipe pipe_in_;

    void store_message_(const std::string& message);

    // Continues until pipe closed/canceled, so we can use RAII
    void async_read_pipe();
    void async_read_pipe(std::shared_ptr<std::string> buffer);

public:
    bp::async_pipe&
    get_pipe()
    {
        return pipe_in_;
    }

    PipeReader();
    PipeReader(const PipeReader&) = delete;
    PipeReader(PipeReader&&) = delete;
    PipeReader& operator=(const PipeReader&) = delete;
    PipeReader& operator=(PipeReader&&) = delete;
    ~PipeReader();

    // Blocking and O(1), use sparingly
    IncomingMessageVariant
    get_message()
    {
        while (true) {
            std::lock_guard<std::mutex> lock{message_lock_};
            if (messages.empty())
                continue;
            auto res = messages.front();
			messages.erase(messages.begin());
            return res;
        }
    }

    std::vector<IncomingMessageVariant>
    get_messages()
    {
        std::lock_guard<std::mutex> lock{message_lock_};
        std::vector<IncomingMessageVariant> result;
        result.swap(messages);
        return result;
    }
};

} // namespace wrappers
} // namespace nutc
