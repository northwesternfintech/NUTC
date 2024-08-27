#pragma once
#include "common/messages_wrapper_to_exchange.hpp"

#include <boost/process.hpp>

#include <mutex>

namespace nutc::exchange {

namespace bp = boost::process;
namespace ba = boost::asio;

class PipeReader {
    std::mutex message_lock_;
    std::vector<common::IncomingMessageVariant> shared;
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
    common::IncomingMessageVariant
    get_message()
    {
        while (true) {
            std::lock_guard<std::mutex> lock{message_lock_};
            if (shared.empty())
                continue;
            auto res = shared.front();
            shared.erase(shared.begin());
            return res;
        }
    }

    std::vector<common::IncomingMessageVariant>
    get_shared()
    {
        std::lock_guard<std::mutex> lock{message_lock_};
        std::vector<common::IncomingMessageVariant> result;
        result.swap(shared);
        return result;
    }
};

} // namespace nutc::exchange
