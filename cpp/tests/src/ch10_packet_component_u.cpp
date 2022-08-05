#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_context.h"
#include "ch10_packet_component.h"

class BitField1
{
   public:
    uint32_t count : 24;
    uint32_t val : 7;
    uint32_t flag : 1;
};

class BitField2
{
   public:
    uint16_t val : 12;
    uint32_t bits1 : 2;
    uint32_t bits2 : 2;
};

class ArrayFloat
{
   public:
    float ff[2];
    ArrayFloat() : ff{-9.0F, 50.1F} {}
};

class Ch10PacketComponentTest : public ::testing::Test
{
   protected:
    ElemPtrVec elems_;
    std::vector<uint8_t> data_;
    size_t total_size_;
    size_t bitfield1_size_;
    size_t bitfield2_size_;
    size_t arrayfloat_size_;
    Ch10Context ctx_;
    Ch10PacketComponent comp_;
    Ch10PacketElement<BitField1> bf1elem_;
    Ch10PacketElement<BitField2> bf2elem_;
    Ch10PacketElement<ArrayFloat> afelem_;

    Ch10PacketComponentTest() : total_size_(0),
                                bitfield1_size_(sizeof(BitField1)),
                                bitfield2_size_(sizeof(BitField2)),
                                arrayfloat_size_(sizeof(ArrayFloat)),
                                ctx_(0, 0),
                                comp_(&ctx_)
    {
        total_size_ = bitfield1_size_ + bitfield2_size_ + arrayfloat_size_;

        // Fill the data_ vector with bits that represent a
        // the series of objects: BitFiel1, BitField2, ArrayFloat.
        data_.resize(total_size_);

        BitField1* bf1ptr = (BitField1*)data_.data();
        bf1ptr->count = 4532;
        bf1ptr->val = 111;
        bf1ptr->flag = 0;

        BitField2* bf2ptr = (BitField2*)(data_.data() + bitfield1_size_);
        bf2ptr->val = 3390;
        bf2ptr->bits1 = 3;
        bf2ptr->bits2 = 1;

        ArrayFloat* afptr = (ArrayFloat*)(data_.data() + bitfield1_size_ + bitfield2_size_);
        afptr->ff[0] = 10.5;
        afptr->ff[1] = 99.0;

        // Create a vector of Ch10PacketElemetBase* pointers
        // for use in automatated parsing.
        elems_.push_back(dynamic_cast<Ch10PacketElementBase*>(
            &bf1elem_));
        elems_.push_back(dynamic_cast<Ch10PacketElementBase*>(
            &bf2elem_));
        elems_.push_back(dynamic_cast<Ch10PacketElementBase*>(
            &afelem_));
    }
};

TEST_F(Ch10PacketComponentTest, ParseElementPointerAreSet)
{
    const uint8_t* data_ptr = (const uint8_t*)data_.data();
    comp_.ParseElements(elems_, data_ptr);

    // If *element is still a nullptr, then don't bother to
    // do anything else.
    ASSERT_TRUE(*bf1elem_.element != nullptr);

    // BitField1 parsed
    EXPECT_EQ((*bf1elem_.element)->count, 4532);
    EXPECT_EQ((*bf1elem_.element)->val, 111);
    EXPECT_EQ((*bf1elem_.element)->flag, 0);

    // Bitfield2 parsed
    EXPECT_EQ((*bf2elem_.element)->val, 3390);
    EXPECT_EQ((*bf2elem_.element)->bits1, 3);
    EXPECT_EQ((*bf2elem_.element)->bits2, 1);

    // ArrauFloat parsed
    EXPECT_EQ((*afelem_.element)->ff[0], 10.5);
    EXPECT_EQ((*afelem_.element)->ff[1], 99.0);

    // data_ptr_ incremented correctly.
    EXPECT_EQ(data_ptr, data_.data() + bf1elem_.size + bf2elem_.size + afelem_.size);
}