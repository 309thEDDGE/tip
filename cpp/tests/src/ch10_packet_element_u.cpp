#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_packet_element.h"

// bitfield
class BitField
{
public:
    uint32_t count  : 24;
    uint32_t val    : 7;
    uint32_t flag   : 1;
};

TEST(Ch10PacketElementTest, SetFloat)
{
    uint64_t pos = 0;
    float f = 30.2;
    Ch10PacketElement<float> elem;
    EXPECT_EQ(sizeof(float), elem.size);
    
    ASSERT_TRUE(*elem.element == nullptr);
    elem.Set((const uint8_t*)&f, pos);
    EXPECT_EQ(pos, sizeof(float));
    ASSERT_TRUE(elem.element != nullptr);
    EXPECT_EQ(**elem.element, f);

    float f2 = 100.0;
    elem.Set((const uint8_t*)&f2, pos);
    EXPECT_EQ(pos, 2*sizeof(float));
    EXPECT_EQ(**elem.element, f2);
}

TEST(Ch10PacketElementTest, SetBitField)
{
    uint64_t pos = 0;
    BitField obj;
    obj.count = 310340;
    obj.val = 21;
    obj.flag = 1;
    Ch10PacketElement<BitField> elem;
    EXPECT_EQ(sizeof(obj), elem.size);
    
    ASSERT_TRUE(*elem.element == nullptr);
    elem.Set((const uint8_t*)&obj, pos);
    EXPECT_EQ(pos, sizeof(obj));
    ASSERT_TRUE(elem.element != nullptr);
    EXPECT_EQ((*elem.element)->count, obj.count);
    EXPECT_EQ((*elem.element)->flag, obj.flag);
    EXPECT_EQ((*elem.element)->val, obj.val);
}
