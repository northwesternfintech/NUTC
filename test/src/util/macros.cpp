#include "macros.hpp"

namespace nutc {
namespace testing_utils {
bool
isNearlyEqual(float a, float b, float epsilon)
{
    float absA = std::fabs(a);
    float absB = std::fabs(b);
    float diff = std::fabs(a - b);
    return diff <= ((absA < absB ? absB : absA) * epsilon);
}

bool
validateMatch(
    const Match& match, const std::string& ticker, const std::string& buyer_uid,
    const std::string& seller_uid, nutc::messages::SIDE side, float price,
    float quantity
)
{
    return match.ticker == ticker && match.buyer_uid == buyer_uid
           && match.seller_uid == seller_uid && match.side == side
           && isNearlyEqual(match.price, price)
           && isNearlyEqual(match.quantity, quantity);
}

bool
validateObUpdate(
    const ObUpdate& update, const std::string& ticker, messages::SIDE side, float price,
    float quantity
)
{
    return update.security == ticker, update.side == side,
           isNearlyEqual(update.price, price), isNearlyEqual(update.quantity, quantity);
}

} // namespace testing_utils
} // namespace nutc
