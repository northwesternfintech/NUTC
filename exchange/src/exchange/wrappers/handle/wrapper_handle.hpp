#pragma once

#include "exchange/wrappers/messaging/pipe_reader.hpp"
#include "exchange/wrappers/messaging/pipe_writer.hpp"

#include <boost/process.hpp>
#include <boost/process/pipe.hpp>

#include <atomic>
#include <filesystem>

namespace nutc {
namespace wrappers {
namespace bp = boost::process;
namespace fs = std::filesystem;

class WrapperHandle {
    bp::child wrapper_;
    PipeReader reader_{};
    PipeWriter writer_{};

    void spawn_wrapper(const std::vector<std::string>& args);

    const fs::path& wrapper_binary_path();

public:
    // Remote (algo in firebase)
    WrapperHandle(const std::string& remote_uid, const std::string& algo_id);

    // Local (.py on disk)
    WrapperHandle(const std::string& algo_path);

    std::vector<std::variant<init_message, market_order>>
    read_messages()
    {
        return reader_.get_messages();
    }

    void
    send_messages(std::vector<std::string> messages)
    {
        return writer_.send_messages(messages);
    }

    ~WrapperHandle();
};
} // namespace wrappers
} // namespace nutc
