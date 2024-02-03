#include "exchange/client_manager/client_manager.hpp"
#include "exchange/matching/engine/engine.hpp"
#include "exchange/utils/logger/logger.hpp"

#include <limits>

using Engine = nutc::matching::Engine;
using MarketOrder = nutc::messages::MarketOrder;
using Logger = nutc::events::Logger;
using ObUpdate = nutc::messages::ObUpdate;
using ClientManager = nutc::manager::ClientManager;
using SIDE = nutc::messages::SIDE;

namespace nutc {
namespace testing_utils {
bool is_nearly_equal(
    float f_a, float f_b, float epsilon = std::numeric_limits<float>::epsilon()
);

bool validate_match(
    const Match& match, const std::string& ticker, const std::string& buyer_id,
    const std::string& seller_id, messages::SIDE side, float price, float quantity
);

bool validate_ob_update(
    const ObUpdate& update, const std::string& ticker, messages::SIDE side, float price,
    float quantity
);

bool validate_market_order(
    const MarketOrder& update, const std::string& client_id, const std::string& ticker,
    messages::SIDE side, float price, float quantity
);

void add_client_simple(manager::ClientManager& manager, const std::string& client_id);

} // namespace testing_utils
} // namespace nutc

#define ASSERT_EQ_MATCH(/* NOLINT(cppcoreguidelines-macro-usage) */                    \
                        match, ticker_, buyer_id_, seller_id_, side_, price_,          \
                        quantity_                                                      \
)                                                                                      \
    do {                                                                               \
        bool isMatchValid = nutc::testing_utils::validate_match(                       \
            (match), (ticker_), (buyer_id_), (seller_id_), (side_), (price_),          \
            (quantity_)                                                                \
        );                                                                             \
        EXPECT_TRUE(isMatchValid)                                                      \
            << "Expected match with ticker = " << (ticker_)                            \
            << ", buyer_id = " << (buyer_id_) << ", seller_id = " << (seller_id_)      \
            << ", side = " << static_cast<int>(side_) << ", price = " << (price_)      \
            << ", quantity = " << (quantity_)                                          \
            << ". Actual match: ticker = " << (match).ticker                           \
            << ", buyer_id = " << (match).buyer_id                                     \
            << ", seller_id = " << (match).seller_id                                   \
            << ", side = " << static_cast<int>((match).side)                           \
            << ", price = " << (match).price << ", quantity = " << (match).quantity;   \
    } while (0)

#define ASSERT_EQ_OB_UPDATE(/* NOLINT(cppcoreguidelines-macro-usage) */                \
                            update, ticker_, side_, price_, quantity_                  \
)                                                                                      \
    do {                                                                               \
        bool isUpdateValid = nutc::testing_utils::validate_ob_update(                  \
            (update), (ticker_), (side_), (price_), (quantity_)                        \
        );                                                                             \
        EXPECT_TRUE(isUpdateValid)                                                     \
            << "Expected update with ticker = " << (ticker_)                           \
            << ", side = " << static_cast<int>(side_) << ", price = " << (price_)      \
            << ", quantity = " << (quantity_)                                          \
            << ". Actual update: ticker = " << (update).ticker                         \
            << ", side = " << static_cast<int>((update).side)                          \
            << ", price = " << (update).price << ", quantity = " << (update).quantity; \
    } while (0)

#define ASSERT_EQ_MARKET_ORDER(/* NOLINT (cppcoreguidelines-macro-usage) */            \
                               update, client_id_, ticker_, side_, price_, quantity_   \
)                                                                                      \
    do {                                                                               \
        bool isUpdateValid = nutc::testing_utils::validate_market_order(               \
            (update), (client_id_), (ticker_), (side_), (price_), (quantity_)          \
        );                                                                             \
        EXPECT_TRUE(isUpdateValid)                                                     \
            << "Expected market order with client_id = " << (client_id_)               \
            << ", ticker =" << (ticker_) << ", side = " << static_cast<int>(side_)     \
            << ", price = " << (price_) << ", quantity = " << (quantity_)              \
            << ". Actual update: ticker = " << (update).ticker                         \
            << ", side = " << static_cast<int>((update).side)                          \
            << ", price = " << (update).price << ", quantity = " << (update).quantity; \
    } while (0)
