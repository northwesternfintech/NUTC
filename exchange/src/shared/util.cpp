#include "util.hpp"

#include "shared/config/config.h"

#include <fmt/format.h>

namespace nutc::shared {
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

} // namespace nutc::shared
