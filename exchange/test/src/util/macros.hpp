#include "common/types/messages/messages_wrapper_to_exchange.hpp"
#include "common/types/ticker.hpp"
#include "exchange/orders/storage/order_storage.hpp"
#include "exchange/traders/trader_container.hpp"

using limit_order = nutc::common::limit_order;
using tagged_limit_order = nutc::exchange::tagged_limit_order;
using tagged_market_order = nutc::exchange::tagged_market_order;
using TraderContainer = nutc::exchange::TraderContainer;

// This is used by GTest to print the value
// DO NOT DELETE OR CHANGE
namespace nutc::common {
void PrintTo(const AlgoLanguage& op, std::ostream* os);
} // namespace nutc::common

namespace nutc::test {

bool validate_match(
    const nutc::exchange::tagged_match& match, common::Ticker ticker,
    const std::string& buyer_id, const std::string& seller_id, common::Side side,
    double quantity, double price
);

bool validate_ob_update(
    const common::position& update, common::Ticker ticker, common::Side side,
    double quantity, double price
);

bool validate_limit_order(
    const limit_order& update, common::Ticker ticker, common::Side side,
    double quantity, double price
);

bool order_equality(const limit_order& order1, const limit_order& order2);

bool
order_equality(const common::market_order& order1, const common::market_order& order2);

} // namespace nutc::test

#define ASSERT_EQ_MATCH(/* NOLINT(cppcoreguidelines-macro-usage) */                    \
                        match, ticker_, buyer_id_, seller_id_, side_, quantity_,       \
                        price_                                                         \
)                                                                                      \
    do {                                                                               \
        bool isMatchValid = nutc::test::validate_match(                                \
            (match), (ticker_), (buyer_id_), (seller_id_), (side_), (quantity_),       \
            (price_)                                                                   \
        );                                                                             \
        EXPECT_TRUE(isMatchValid)                                                      \
            << "Expected match with ticker = " << (nutc::common::to_string(ticker_))   \
            << ", buyer_id = " << (buyer_id_) << ", seller_id = " << (seller_id_)      \
            << ", side = " << static_cast<int>(side_) << ", price = " << (price_)      \
            << ", quantity = " << (quantity_)                                          \
            << ". Actual match: ticker = " << nutc::common::to_string((match).ticker)  \
            << ", buyer_id = " << (match).buyer->get_id()                              \
            << ", seller_id = " << (match).seller->get_id()                            \
            << ", side = " << static_cast<int>((match).side)                           \
            << ", price = " << double{(match).price}                                   \
            << ", quantity = " << double{(match).quantity};                            \
    } while (0)

#define ASSERT_EQ_OB_UPDATE(/* NOLINT(cppcoreguidelines-macro-usage) */                \
                            update, ticker_, side_, quantity_, price_                  \
)                                                                                      \
    do {                                                                               \
        bool isUpdateValid = nutc::test::validate_ob_update(                           \
            (update), (ticker_), (side_), (quantity_), (price_)                        \
        );                                                                             \
        EXPECT_TRUE(isUpdateValid)                                                     \
            << "Expected update with ticker = " << (nutc::common::to_string(ticker_))  \
            << ", side = " << static_cast<int>(side_) << ", price = " << (price_)      \
            << ", quantity = " << (quantity_) << ". Actual update: ticker = "          \
            << nutc::common::to_string((update).ticker)                                \
            << ", side = " << static_cast<int>((update).side)                          \
            << ", price = " << double((update).price)                                  \
            << ", quantity = " << double{(update).quantity};                           \
    } while (0)

#define ASSERT_EQ_LIMIT_ORDER(/* NOLINT (cppcoreguidelines-macro-usage) */             \
                              update, ticker_, side_, quantity_, price_                \
)                                                                                      \
    do {                                                                               \
        bool isUpdateValid = nutc::test::validate_limit_order(                         \
            (update), (ticker_), (side_), (quantity_), (price_)                        \
        );                                                                             \
        EXPECT_TRUE(isUpdateValid)                                                     \
            << "Expected market order with"                                            \
            << " ticker =" << (nutc::common::to_string(ticker_))                       \
            << ", side = " << static_cast<int>(side_) << ", price = " << (price_)      \
            << ", quantity = " << (quantity_) << ". Actual update: client_id = " << "" \
            << ", ticker = " << nutc::common::to_string((update).ticker)               \
            << ", side = " << static_cast<int>((update).side)                          \
            << ", price = " << double((update).price)                                  \
            << ", quantity = " << double{(update).quantity};                           \
    } while (0)
