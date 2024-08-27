#include "util.hpp"

#include "common/config/config.h"

#include <fmt/format.h>

#ifdef __APPLE__
#  include <mach/mach_time.h>
#else
#  include <x86intrin.h>
#endif

namespace nutc::common {
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

std::string
trader_id(const std::string& user_id, const std::string& algo_id)
{
    return fmt::format("{}-{}", user_id, algo_id);
}

} // namespace nutc::common
