#include "exchange/orders/matching/engine.hpp"
#include "exchange/orders/storage/order_storage.hpp"
#include "exchange/traders/trader_container.hpp"
#include "exchange/traders/trader_types/generic_trader.hpp"
#include "shared/messages_exchange_to_wrapper.hpp"
#include "shared/messages_wrapper_to_exchange.hpp"
#include "shared/ticker.hpp"

#include <limits>

using Engine = nutc::matching::Engine;
using market_order = nutc::messages::market_order;
using stored_order = nutc::matching::stored_order;
using orderbook_update = nutc::messages::orderbook_update;
using TraderContainer = nutc::traders::TraderContainer;

namespace nutc {
namespace test_utils {

market_order consume_message(const std::shared_ptr<traders::GenericTrader>& trader);

bool is_nearly_equal(
    double f_a, double f_b, double epsilon = std::numeric_limits<double>::epsilon()
);

bool validate_match(
    const nutc::matching::stored_match& match, util::Ticker ticker,
    const std::string& buyer_id, const std::string& seller_id, util::Side side,
    double price, double quantity
);

bool validate_ob_update(
    const orderbook_update& update, util::Ticker ticker, util::Side side, double price,
    double quantity
);

bool validate_market_order(
    const market_order& update, util::Ticker ticker, util::Side side, double price,
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
            << ". Actual match: ticker = " << std::string{(match).ticker}              \
            << ", buyer_id = " << (match).buyer.get_id()                               \
            << ", seller_id = " << (match).seller.get_id()                             \
            << ", side = " << static_cast<int>((match).side)                           \
            << ", price = " << (match).price << ", quantity = " << (match).quantity;   \
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

#define ASSERT_EQ_MARKET_ORDER(/* NOLINT (cppcoreguidelines-macro-usage) */            \
                               update, ticker_, side_, price_, quantity_               \
)                                                                                      \
    do {                                                                               \
        bool isUpdateValid = nutc::test_utils::validate_market_order(                  \
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
