#pragma once

#include "exchange/wrappers/messaging/pipe_reader.hpp"
#include "exchange/wrappers/messaging/pipe_writer.hpp"

#include <boost/process.hpp>
#include <boost/process/pipe.hpp>

#include <filesystem>

namespace nutc {
namespace wrappers {
namespace bp = boost::process;
namespace fs = std::filesystem;

class WrapperHandle {
    bp::child wrapper_;
    PipeReader reader_{};
    PipeWriter writer_{};

    WrapperHandle(const std::vector<std::string>& args, const std::string& algorithm);
    void block_on_init();

    const fs::path& wrapper_binary_path();

public:
    /* Both constructors will block on an init messages, ensuring proper construction
     * There should *not* be a case (including errors in submitted code) where the
     * wrapper does not send an init_message. this MUST happen
     */

    // Remote (algo in firebase)
    WrapperHandle(const std::string& remote_uid, const std::string& algo_id);

    // Local (.py on disk)
    WrapperHandle(const std::string& algo_path);

    using ReadMessageVariant = std::variant<init_message, limit_order, market_order>;
    std::vector<ReadMessageVariant> read_messages();

    void
    send_message(const std::string& message)
    {
        return writer_.send_message(message);
    }

    ~WrapperHandle();
};
} // namespace wrappers
} // namespace nutc
