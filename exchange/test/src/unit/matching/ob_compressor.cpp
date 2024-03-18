#include "exchange/tickers/engine/ob_compressor.hpp"

#include <gtest/gtest.h>
using nutc::matching::OrderbookCompressor;
using nutc::messages::ObUpdate;
using nutc::messages::SIDE;

TEST(UnitObCompression, BasicDisjointAdd)
{
    OrderbookCompressor compressor = OrderbookCompressor("ETH");
    compressor.add_update(ObUpdate{"ETH", SIDE::BUY, 1, 1});
    compressor.add_update(ObUpdate{"ETH", SIDE::SELL, 2, 1});
    compressor.add_update(ObUpdate{"ETH", SIDE::SELL, 3, 2});

    std::vector<ObUpdate> compressed = compressor.compress_and_get_updates();
    ASSERT_EQ(compressed.size(), 3);

    std::vector<ObUpdate> expected = {
        ObUpdate{"ETH", SIDE::BUY,  1, 1},
        ObUpdate{"ETH", SIDE::SELL, 3, 2},
        ObUpdate{"ETH", SIDE::SELL, 2, 1}
    };

    for (size_t i = 0; i < compressed.size(); i++) {
        ASSERT_EQ(compressed[i].ticker, expected[i].ticker);
        ASSERT_EQ(compressed[i].side, expected[i].side);
        ASSERT_EQ(compressed[i].price, expected[i].price);
        ASSERT_EQ(compressed[i].quantity, expected[i].quantity);
    }
}

TEST(UnitObCompression, BasicAdd)
{
    OrderbookCompressor compressor = OrderbookCompressor("ETH");
    compressor.add_update(ObUpdate{"ETH", SIDE::BUY, 1, 1});
    compressor.add_update(ObUpdate{"ETH", SIDE::SELL, 2, 1});
    compressor.add_update(ObUpdate{"ETH", SIDE::SELL, 2, 2});

    std::vector<ObUpdate> compressed = compressor.compress_and_get_updates();
    EXPECT_EQ(compressed.size(), 2);

    std::vector<ObUpdate> expected = {
        ObUpdate{"ETH", SIDE::BUY,  1, 1},
        ObUpdate{"ETH", SIDE::SELL, 2, 3}
    };

    for (size_t i = 0; i < compressed.size(); i++) {
        ASSERT_EQ(compressed[i].ticker, expected[i].ticker);
        ASSERT_EQ(compressed[i].side, expected[i].side);
        ASSERT_EQ(compressed[i].price, expected[i].price);
        ASSERT_EQ(compressed[i].quantity, expected[i].quantity);
    }
}

TEST(UnitObCompression, MultiAdd)
{
    OrderbookCompressor compressor = OrderbookCompressor("ETH");
    compressor.add_update(ObUpdate{"ETH", SIDE::SELL, 1, 2});
    compressor.add_update(ObUpdate{"ETH", SIDE::BUY, 1, 1});
    compressor.add_update(ObUpdate{"ETH", SIDE::SELL, 2, 1});
    compressor.add_update(ObUpdate{"ETH", SIDE::SELL, 2, 2});

    std::vector<ObUpdate> compressed = compressor.compress_and_get_updates();
    EXPECT_EQ(compressed.size(), 3);

    std::vector<ObUpdate> expected = {
        ObUpdate{"ETH", SIDE::BUY,  1, 1},
        ObUpdate{"ETH", SIDE::SELL, 2, 3},
        ObUpdate{"ETH", SIDE::SELL, 1, 2}
    };

    for (size_t i = 0; i < compressed.size(); i++) {
        ASSERT_EQ(compressed[i].ticker, expected[i].ticker);
        ASSERT_EQ(compressed[i].side, expected[i].side);
        ASSERT_EQ(compressed[i].price, expected[i].price);
        ASSERT_EQ(compressed[i].quantity, expected[i].quantity);
    }
}
