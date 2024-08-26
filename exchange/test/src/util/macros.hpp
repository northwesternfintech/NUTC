#include "exchange/matching/engine.hpp"
#include "exchange/orders/storage/order_storage.hpp"
#include "exchange/traders/trader_container.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"
#include "shared/types/ticker.hpp"

using Engine = nutc::exchange::Engine;
using limit_order = nutc::shared::limit_order;
using tagged_limit_order = nutc::exchange::tagged_limit_order;
using TraderContainer = nutc::exchange::TraderContainer;

namespace nutc::test {

bool is_nearly_equal(double f_a, double f_b);

bool validate_match(
    const nutc::shared::match& match, shared::Ticker ticker,
    const std::string& buyer_id, const std::string& seller_id, shared::Side side,
    double quantity, double price
);

bool validate_ob_update(
    const shared::position& update, shared::Ticker ticker, shared::Side side,
    double quantity, double price
);

bool validate_limit_order(
    const limit_order& update, shared::Ticker ticker, shared::Side side,
    double quantity, double price
);

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
            << "Expected match with ticker = " << (nutc::shared::to_string(ticker_))   \
            << ", buyer_id = " << (buyer_id_) << ", seller_id = " << (seller_id_)      \
            << ", side = " << static_cast<int>(side_) << ", price = " << (price_)      \
            << ", quantity = " << (quantity_) << ". Actual match: ticker = "           \
            << nutc::shared::to_string((match).position.ticker)                        \
            << ", buyer_id = " << (match).buyer_id                                     \
            << ", seller_id = " << (match).seller_id                                   \
            << ", side = " << static_cast<int>((match).position.side)                  \
            << ", price = " << double{(match).position.price}                          \
            << ", quantity = " << (match).position.quantity;                           \
    } while (0)

#define ASSERT_EQ_OB_UPDATE(/* NOLINT(cppcoreguidelines-macro-usage) */                \
                            update, ticker_, side_, quantity_, price_                  \
)                                                                                      \
    do {                                                                               \
        bool isUpdateValid = nutc::test::validate_ob_update(                           \
            (update), (ticker_), (side_), (quantity_), (price_)                        \
        );                                                                             \
        EXPECT_TRUE(isUpdateValid)                                                     \
            << "Expected update with ticker = " << (nutc::shared::to_string(ticker_))  \
            << ", side = " << static_cast<int>(side_) << ", price = " << (price_)      \
            << ", quantity = " << (quantity_) << ". Actual update: ticker = "          \
            << nutc::shared::to_string((update).ticker)                                \
            << ", side = " << static_cast<int>((update).side)                          \
            << ", price = " << double((update).price)                                  \
            << ", quantity = " << (update).quantity;                                   \
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
            << " ticker =" << (nutc::shared::to_string(ticker_))                       \
            << ", side = " << static_cast<int>(side_) << ", price = " << (price_)      \
            << ", quantity = " << (quantity_) << ". Actual update: client_id = "       \
            << ""                                                                      \
            << ", ticker = " << nutc::shared::to_string((update).ticker)               \
            << ", side = " << static_cast<int>((update).side)                          \
            << ", price = " << double((update).price)                                  \
            << ", quantity = " << (update).quantity;                                   \
    } while (0)
