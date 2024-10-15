#include "util.hpp"

#include "common/config/config.h"

#include <boost/algorithm/string.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <fmt/format.h>

#include <atomic>

#ifdef __APPLE__
#  include <mach/mach_time.h>
#else
#  include <x86intrin.h>
#endif

namespace nutc::common {
namespace bi = boost::archive::iterators;

order_id_t
generate_order_id()
{
    // static constexpr auto MAX_PID_BITS = 22;
    static auto pid = static_cast<std::int64_t>(getpid());
    // static std::atomic<std::int64_t> start_order_id{(pid << 46)};
	static std::int64_t start_order_id{(pid<<46)};
    return ++start_order_id;
}

uint64_t
get_time()
{
#ifdef __APPLE__
    static uint64_t min_time = 0;
    return min_time = std::max(min_time + 1, mach_absolute_time());
#else
    return __rdtsc();
#endif
}

std::string
get_firebase_endpoint(const std::string& params)
{
#ifdef NUTC_LOCAL_DEV
    return FIREBASE_URL + params + "?ns=nutc-web-default-rtdb";
#else
    return FIREBASE_URL + params;
#endif
}

std::string
to_string(Side side)
{
    return side == Side::buy ? "BUY" : "SELL";
}

// https://stackoverflow.com/questions/7053538/how-do-i-encode-a-string-to-base64-using-only-boost
std::string
base64_encode(const std::string& data)
{
    using base64_it =
        bi::base64_from_binary<bi::transform_width<std::string::const_iterator, 6, 8>>;

    auto tmp = std::string(base64_it(data.begin()), base64_it(data.end()));
    return tmp.append((3 - data.size() % 3) % 3, '=');
}

std::string
base64_decode(const std::string& data)
{
    using base64_it =
        bi::transform_width<bi::binary_from_base64<std::string::const_iterator>, 8, 6>;

    return boost::algorithm::trim_right_copy_if(
        std::string(base64_it(data.begin()), base64_it(data.end())),
        [](char chr) { return chr == '\0'; }
    );
}

std::string
trader_id(const std::string& user_id, const std::string& algo_id)
{
    return fmt::format("{}-{}", user_id, algo_id);
}

} // namespace nutc::common
