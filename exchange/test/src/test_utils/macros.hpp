#include "matching/engine/engine.hpp"

#include <cmath>

#include <limits>

using Engine = nutc::matching::Engine;
using MarketOrder = nutc::messages::MarketOrder;
using ObUpdate = nutc::messages::ObUpdate;
using ClientManager = nutc::manager::ClientManager;
using SIDE = nutc::messages::SIDE;

namespace nutc {
namespace testing_utils {
bool
isNearlyEqual(float a, float b, float epsilon = std::numeric_limits<float>::epsilon());

bool validateMatch(
    const Match& match, const std::string& ticker, const std::string& buyer_uid,
    const std::string& seller_uid, messages::SIDE side, float price, float quantity
);

bool validateObUpdate(
    const ObUpdate& update, const std::string& ticker, messages::SIDE side, float price,
    float quantity
);

} // namespace testing_utils
} // namespace nutc

#define EXPECT_EQ_MATCH(                                                               \
    match, ticker_, buyer_uid_, seller_uid_, side_, price_, quantity_                  \
)                                                                                      \
    do {                                                                               \
        bool isMatchValid = nutc::testing_utils::validateMatch(                              \
            (match), (ticker_), (buyer_uid_), (seller_uid_), (side_), (price_),        \
            (quantity_)                                                                \
        );                                                                             \
        EXPECT_TRUE(isMatchValid)                                                      \
            << "Expected match with ticker = " << (ticker_)                            \
            << ", buyer_uid = " << (buyer_uid_) << ", seller_uid = " << (seller_uid_)  \
            << ", side = " << static_cast<int>(side_) << ", price = " << (price_)      \
            << ", quantity = " << (quantity_)                                          \
            << ". Actual match: ticker = " << (match).ticker                           \
            << ", buyer_uid = " << (match).buyer_uid                                   \
            << ", seller_uid = " << (match).seller_uid                                 \
            << ", side = " << static_cast<int>((match).side)                           \
            << ", price = " << (match).price << ", quantity = " << (match).quantity;   \
    } while (0)

#define EXPECT_EQ_OB_UPDATE(update, ticker_, side_, price_, quantity_)                 \
    do {                                                                               \
        bool isUpdateValid = nutc::testing_utils::validateObUpdate(                          \
            (update), (ticker_), (side_), (price_), (quantity_)                        \
        );                                                                             \
        EXPECT_TRUE(isUpdateValid)                                                     \
            << "Expected update with ticker = " << (ticker_)                           \
            << ", side = " << static_cast<int>(side_) << ", price = " << (price_)      \
            << ", quantity = " << (quantity_)                                          \
            << ". Actual update: ticker = " << (update).security                       \
            << ", side = " << static_cast<int>((update).side)                          \
            << ", price = " << (update).price << ", quantity = " << (update).quantity; \
    } while (0)
