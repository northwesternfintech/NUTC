#include "exchange/matching/engine.hpp"
#include "exchange/orders/storage/order_storage.hpp"
#include "exchange/traders/trader_container.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"
#include "shared/types/ticker.hpp"

using Engine = nutc::matching::Engine;
using limit_order = nutc::messages::limit_order;
using stored_order = nutc::matching::stored_order;
using TraderContainer = nutc::traders::TraderContainer;

namespace nutc {
namespace test_utils {

bool is_nearly_equal(double f_a, double f_b);

bool validate_match(
    const nutc::matching::stored_match& match, util::Ticker ticker,
    const std::string& buyer_id, const std::string& seller_id, util::Side side,
    double price, double quantity
);

bool validate_ob_update(
    const util::position& update, util::Ticker ticker, util::Side side, double price,
    double quantity
);

bool validate_limit_order(
    const limit_order& update, util::Ticker ticker, util::Side side, double price,
    double quantity
);

} // namespace test_utils
} // namespace nutc

#define ASSERT_EQ_MATCH(/* NOLINT(cppcoreguidelines-macro-usage) */                    \
                        match, ticker_, buyer_id_, seller_id_, side_, price_,          \
                        quantity_                                                      \
)                                                                                      \
    do {                                                                               \
        bool isMatchValid = nutc::test_utils::validate_match(                          \
            (match), (ticker_), (buyer_id_), (seller_id_), (side_), (price_),          \
            (quantity_)                                                                \
        );                                                                             \
        EXPECT_TRUE(isMatchValid)                                                      \
            << "Expected match with ticker = " << (ticker_)                            \
            << ", buyer_id = " << (buyer_id_) << ", seller_id = " << (seller_id_)      \
            << ", side = " << static_cast<int>(side_) << ", price = " << (price_)      \
            << ", quantity = " << (quantity_)                                          \
            << ". Actual match: ticker = " << std::string{(match).position.ticker}     \
            << ", buyer_id = " << (match).buyer.get_id()                               \
            << ", seller_id = " << (match).seller.get_id()                             \
            << ", side = " << static_cast<int>((match).position.side)                  \
            << ", price = " << (match).position.price                                  \
            << ", quantity = " << (match).position.quantity;                           \
    } while (0)

#define ASSERT_EQ_OB_UPDATE(/* NOLINT(cppcoreguidelines-macro-usage) */                \
                            update, ticker_, side_, price_, quantity_                  \
)                                                                                      \
    do {                                                                               \
        bool isUpdateValid = nutc::test_utils::validate_ob_update(                     \
            (update), (ticker_), (side_), (price_), (quantity_)                        \
        );                                                                             \
        EXPECT_TRUE(isUpdateValid)                                                     \
            << "Expected update with ticker = " << (ticker_)                           \
            << ", side = " << static_cast<int>(side_) << ", price = " << (price_)      \
            << ", quantity = " << (quantity_)                                          \
            << ". Actual update: ticker = " << std::string{(update).ticker}            \
            << ", side = " << static_cast<int>((update).side)                          \
            << ", price = " << (update).price << ", quantity = " << (update).quantity; \
    } while (0)

#define ASSERT_EQ_LIMIT_ORDER(/* NOLINT (cppcoreguidelines-macro-usage) */             \
                              update, ticker_, side_, price_, quantity_                \
)                                                                                      \
    do {                                                                               \
        bool isUpdateValid = nutc::test_utils::validate_limit_order(                   \
            (update), (ticker_), (side_), (price_), (quantity_)                        \
        );                                                                             \
        EXPECT_TRUE(isUpdateValid)                                                     \
            << "Expected market order with"                                            \
            << " ticker =" << (ticker_) << ", side = " << static_cast<int>(side_)      \
            << ", price = " << (price_) << ", quantity = " << (quantity_)              \
            << ". Actual update: client_id = "                                         \
            << ""                                                                      \
            << ", ticker = " << std::string{(update).ticker}                           \
            << ", side = " << static_cast<int>((update).side)                          \
            << ", price = " << (update).price << ", quantity = " << (update).quantity; \
    } while (0)
