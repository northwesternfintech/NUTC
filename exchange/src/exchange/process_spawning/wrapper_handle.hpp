#pragma once

#include <boost/process.hpp>
#include <boost/process/pipe.hpp>

namespace nutc {
namespace spawning {
namespace bp = boost::process;

class WrapperHandle {
    bp::child wrapper_{};
    bp::opstream pipe_to_wrapper_{};

public:
    WrapperHandle() = default;

    WrapperHandle(bp::child&& wrapper, bp::opstream&& to_wrapper) :
        wrapper_(std::move(wrapper)), pipe_to_wrapper_(std::move(to_wrapper))
    {}

    void
    send_message(const std::string& message)
    {
        if (pipe_to_wrapper_.good())
            pipe_to_wrapper_ << message << std::endl;
    }

    ~WrapperHandle()
    {
        if (wrapper_.running())
            wrapper_.terminate();
        pipe_to_wrapper_.close();
    }

    WrapperHandle(WrapperHandle&& other) :
        wrapper_(std::move(other.wrapper_)),
        pipe_to_wrapper_(std::move(other.pipe_to_wrapper_))
    {}

    WrapperHandle&
    operator=(WrapperHandle&& other)
    {
        wrapper_ = std::move(other.wrapper_);
        pipe_to_wrapper_ = std::move(other.pipe_to_wrapper_);
        return *this;
    }
};
} // namespace spawning
} // namespace nutc
