#include "common/types/algorithm/algorithm.hpp"
#include "exchange/wrappers/messaging/pipe_reader.hpp"
#include "exchange/wrappers/messaging/pipe_writer.hpp"

#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/process/pipe.hpp>

namespace nutc::exchange {

namespace bp = boost::process;

class WrapperHandle {
    bp::child wrapper_;
    PipeReader reader_;
    PipeWriter writer_;

    WrapperHandle(const std::vector<std::string>& args, const std::string& algo_string);
    void block_on_init();

public:
    /* Both constructors will block on an init shared, ensuring proper construction
     * There should *not* be a case (including errors in submitted code) where the
     * wrapper does not send an init_message. this MUST happen
     */

    explicit WrapperHandle(const common::algorithm_variant& algo_variant);

    std::vector<common::IncomingMessageVariant>
    read_shared()
    {
        return reader_.get_shared();
    }

    void
    send_message(const std::string& message)
    {
        return writer_.send_message(message);
    }

    ~WrapperHandle();

private:
    static std::vector<std::string>
    create_arguments(const common::algorithm_variant& algo_variant);
};
} // namespace nutc::exchange
