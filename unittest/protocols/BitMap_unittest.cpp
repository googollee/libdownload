#include "protocols/http/BitMap.h"

#include <gtest/gtest.h>

TEST(BitMapTest, TestZero)
{
    BitMap map;

    EXPECT_EQ(map.size(), 0u);
    EXPECT_EQ(map.bytesPerBit(), 0u);
}

TEST(BitMapTest, Test3000Size1Byte_ctor)
{
    BitMap map(3000, 1);

    EXPECT_EQ(map.size(), 3000u);
    EXPECT_EQ(map.bytesPerBit(), 1u);

    for (size_t i=0; i<map.size(); ++i)
    {
        EXPECT_EQ(map.get(i), false);
    }
}

TEST(BitMapTest, Test3000Size1Byte_set)
{
    BitMap map(3000, 1);

    map.set(12, true);
    for (size_t i=0; i<map.size(); ++i)
    {
        if (i != 12)
            EXPECT_EQ(map.get(i), false);
        else
            EXPECT_EQ(map.get(i), true);
    }

    EXPECT_EQ(map.find(true), 12u);
    EXPECT_EQ(map.find(true, 10), 12u);
}

TEST(BitMapTest, Test3000Size1Byte_setAll)
{
    BitMap map(3000, 1);

    map.setAll(true);
    for (size_t i=0; i<map.size(); ++i)
    {
        EXPECT_EQ(map.get(i), true);
    }
}

TEST(BitMapTest, Test3000Size1Byte_setRange)
{
    BitMap map(3000, 1);

    map.setAll(false);
    map.setRange(0, 100, true);
    for (size_t i=0; i<map.size(); ++i)
    {
        if (i < 100)
            EXPECT_EQ(map.get(i), true);
        else
            EXPECT_EQ(map.get(i), false);
    }

    map.setAll(false);
    map.setRange(35, 99, true);
    for (size_t i=0; i<map.size(); ++i)
    {
        if (35 <= i && i < 99)
            EXPECT_EQ(map.get(i), true);
        else
            EXPECT_EQ(map.get(i), false);
    }

}

TEST(BitMapTest, Test3000Size1Byte_getPositionByLength)
{
    BitMap map(3000, 1);

    map.setAll(false);
    EXPECT_EQ(map.getPositionByLength(0), 0u);
    EXPECT_EQ(map.getPositionByLength(1), 1u);
    EXPECT_EQ(map.getPositionByLength(2999), 2999u);
}

TEST(BitMapTest, Test3000Size1Byte_setRangeByLength)
{
    BitMap map(3000, 1);

    map.setAll(false);
    map.setRangeByLength(0, 100, true);
    for (size_t i=0; i<map.size(); ++i)
    {
        if (i < 100)
            EXPECT_EQ(map.get(i), true);
        else
            EXPECT_EQ(map.get(i), false);
    }

    map.setAll(false);
    map.setRange(35, 99, true);
    for (size_t i=0; i<map.size(); ++i)
    {
        if (35 <= i && i < 99)
        {
            EXPECT_EQ(map.get(i), true);
        }
        else
            EXPECT_EQ(map.get(i), false);
    }
}

TEST(BitMapTest, Test300Size10Byte_ctor)
{
    BitMap map(300 * 10, 10);

    EXPECT_EQ(map.size(), 300u);
    EXPECT_EQ(map.bytesPerBit(), 10u);

    for (size_t i=0; i<map.size(); ++i)
    {
        EXPECT_EQ(map.get(i), false);
    }
}

TEST(BitMapTest, Test300Size10Byte_set)
{
    BitMap map(300 * 10, 10);

    map.set(12, true);
    for (size_t i=0; i<map.size(); ++i)
    {
        if (i != 12)
            EXPECT_EQ(map.get(i), false);
        else
            EXPECT_EQ(map.get(i), true);
    }

    EXPECT_EQ(map.find(true), 12u);
    EXPECT_EQ(map.find(true, 10), 12u);
}

TEST(BitMapTest, Test300Size1Byte_setAll)
{
    BitMap map(300 * 10, 10);

    map.setAll(true);
    for (size_t i=0; i<map.size(); ++i)
    {
        EXPECT_EQ(map.get(i), true);
    }
}

TEST(BitMapTest, Test300Size10Byte_setRange)
{
    BitMap map(300 * 10, 10);

    map.setAll(false);
    map.setRange(0, 100, true);
    for (size_t i=0; i<map.size(); ++i)
    {
        if (i < 100)
            EXPECT_EQ(map.get(i), true);
        else
            EXPECT_EQ(map.get(i), false);
    }

    map.setAll(false);
    map.setRange(35, 99, true);
    for (size_t i=0; i<map.size(); ++i)
    {
        if (35 <= i && i < 99)
            EXPECT_EQ(map.get(i), true);
        else
            EXPECT_EQ(map.get(i), false);
    }

}

TEST(BitMapTest, Test3000Size10Byte_setRangeByLength)
{
    BitMap map(300 * 10, 10);

    map.setAll(false);
    map.setRangeByLength(0, 1000, true);
    for (size_t i=0; i<map.size(); ++i)
    {
        if (i < 100)
            EXPECT_EQ(map.get(i), true);
        else
            EXPECT_EQ(map.get(i), false);
    }

    map.setAll(false);
    map.setRangeByLength(350, 990, true);
    for (size_t i=0; i<map.size(); ++i)
    {
        if (35 <= i && i < 99)
        {
            EXPECT_EQ(map.get(i), true);
        }
        else
            EXPECT_EQ(map.get(i), false);
    }

    map.setAll(false);
    map.setRangeByLength(359, 999, true);
    for (size_t i=0; i<map.size(); ++i)
    {
        if (35 <= i && i < 99)
        {
            EXPECT_EQ(map.get(i), true);
        }
        else
            EXPECT_EQ(map.get(i), false);
    }
}

TEST(BitMapTest, Test300Size10Byte_getPositionByLength)
{
    BitMap map(300 * 10, 10);

    map.setAll(false);
    EXPECT_EQ(map.getPositionByLength(0), 0u);
    EXPECT_EQ(map.getPositionByLength(1), 0u);
    EXPECT_EQ(map.getPositionByLength(9), 0u);
    EXPECT_EQ(map.getPositionByLength(10), 1u);
    EXPECT_EQ(map.getPositionByLength(11), 1u);
    EXPECT_EQ(map.getPositionByLength(2999), 299u);
}

TEST(BitMapTest, TestSetLastBitByRange)
{
    BitMap map(3000, 13);

    map.setAll(false);
    EXPECT_EQ(map.size(), 231u);

    map.setRangeByLength(0, 3000, true);
    for (size_t i=0; i<map.size(); ++i)
    {
        EXPECT_EQ(map.get(i), true);
    }
}
