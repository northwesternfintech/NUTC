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

    WrapperHandle(
        const std::vector<std::string>& args,
        const std::string& algorithm
    );
    void block_on_init();

    const fs::path& wrapper_binary_path();

public:
    /* Both constructors will block on an init messages, ensuring proper construction
     * There should *not* be a case (incl submitted code errors) where the wrapper does
     * not send an init_message
     */

    // Remote (algo in firebase)
    WrapperHandle(const std::string& remote_uid, const std::string& algo_id);

    // Local (.py on disk)
    WrapperHandle(const std::string& algo_path);

    inline
    std::string
    force_upwrap_optional(std::optional<std::string> opt, std::string error_msg) {
        if (!opt.has_value()) {
            throw std::runtime_error(error_msg);
        }

        return opt.value();
    }

    std::vector<limit_order>
    read_messages()
    {
        auto messages = reader_.get_messages();
        std::vector<limit_order> orders{};
        orders.reserve(messages.size());

        if (std::ranges::any_of(messages, [](auto&& mess) {
                return std::holds_alternative<init_message>(mess);
            }))
            throw std::runtime_error("Unexpected init message");

        for (const auto& message : messages) {
            orders.push_back(std::move(std::get<limit_order>(message)));
        }
        return orders;
    }

    void
    send_message(const std::string& message)
    {
        return writer_.send_message(message);
    }

    ~WrapperHandle();
};
} // namespace wrappers
} // namespace nutc
