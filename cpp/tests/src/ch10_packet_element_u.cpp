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
    float f = 30.2;
    Ch10PacketElement<float> elem;
    EXPECT_EQ(sizeof(float), elem.size);
    
    ASSERT_TRUE(*elem.element == nullptr);
    elem.Set((const uint8_t*)&f);
    ASSERT_TRUE(elem.element != nullptr);
    EXPECT_EQ(**elem.element, f);

    float f2 = 100.0;
    elem.Set((const uint8_t*)&f2);
    EXPECT_EQ(**elem.element, f2);
}

TEST(Ch10PacketElementTest, SetBitField)
{
    BitField obj;
    obj.count = 310340;
    obj.val = 21;
    obj.flag = 1;
    Ch10PacketElement<BitField> elem;
    EXPECT_EQ(sizeof(obj), elem.size);
    
    ASSERT_TRUE(*elem.element == nullptr);
    elem.Set((const uint8_t*)&obj);
    ASSERT_TRUE(elem.element != nullptr);
    EXPECT_EQ((*elem.element)->count, obj.count);
    EXPECT_EQ((*elem.element)->flag, obj.flag);
    EXPECT_EQ((*elem.element)->val, obj.val);
}
