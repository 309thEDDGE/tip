#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_videof0_component.h"

class Ch10VideoF0ComponentTest : public ::testing::Test
{
protected:
    Ch10VideoF0Component component_;
    Ch10PacketHeaderFmt packet_header_;
    Ch10Context context_;
    Ch10VideoF0HeaderFormat csdw_;
    Ch10PacketElement<Ch10VideoF0HeaderFormat> csdw_element;

    Ch10VideoF0ComponentTest() : component_(&context_)
     {
         packet_header_.chanID = 4;
         packet_header_.data_size = 55;
         packet_header_.pkt_size = 80;
         context_.UpdateContext(0, &packet_header_);

         csdw_.BA = 0;
         csdw_.PL = 1;
         csdw_.KLV = 1;
         csdw_.SRS = 1;
         csdw_.IPH = 1;
         csdw_.ET = 1;
     }
};

TEST_F(Ch10VideoF0ComponentTest, ParseAvancesDataPointer)
{
    Ch10Status status;
    uint8_t* data_ptr = (uint8_t*)&csdw_;
    uint8_t* expected = data_ptr + sizeof(csdw_);

    status = component_.Parse((const uint8_t* &)data_ptr);

    EXPECT_EQ(data_ptr, expected);
}

TEST_F(Ch10VideoF0ComponentTest, ParseCreatesCsdwElement)
{
    Ch10Status status;
    uint8_t* data_ptr = (uint8_t*)&csdw_;
    uint8_t* expected = data_ptr;

    status = component_.Parse((const uint8_t* &)data_ptr);

    // EXPECT_EQ((uint8_t*)component_.csdw_element.element, expected);
    EXPECT_EQ((*component_.csdw_element.element)->BA, csdw_.BA);
    EXPECT_EQ((*component_.csdw_element.element)->PL, csdw_.PL);
    EXPECT_EQ((*component_.csdw_element.element)->KLV, csdw_.KLV);
    EXPECT_EQ((*component_.csdw_element.element)->SRS, csdw_.SRS);
    EXPECT_EQ((*component_.csdw_element.element)->IPH, csdw_.IPH);
    EXPECT_EQ((*component_.csdw_element.element)->ET, csdw_.ET);
}

void ValidateSubpacketCount(
    Ch10PacketHeaderFmt packet_header, 
    Ch10VideoF0HeaderFormat csdw, 
    Ch10Context &context, 
    Ch10VideoF0Component component)
{
    Ch10Status status;
    uint8_t *data_ptr;
    uint8_t subpacket_size = TransportStream_UNIT_SIZE;

    if (csdw.IPH) subpacket_size += context.intrapacket_ts_size_;

    // Test integer (complete) packet count
    packet_header.data_size = 5 * subpacket_size;
    context.UpdateContext(0, &packet_header);

    data_ptr = (uint8_t*)&csdw;
    status = component.Parse((const uint8_t* &)data_ptr);
    EXPECT_EQ(Ch10Status::OK, status);

    // Test noninteger (incomplete) packet count
    packet_header.data_size -= 1;
    context.UpdateContext(0, &packet_header);

    data_ptr = (uint8_t*)&csdw;
    status = component.Parse((const uint8_t* &)data_ptr);
    EXPECT_EQ(Ch10Status::VIDEOF0_NONINTEGER_SUBPKT_COUNT, status);
}

TEST_F(Ch10VideoF0ComponentTest, ParseRejectsNonintegerSubpacketCountWithIph)
{
    csdw_.IPH = 1;
    ValidateSubpacketCount(packet_header_, csdw_, context_, component_);
}

TEST_F(Ch10VideoF0ComponentTest, ParseRejectsNonintegerSubpacketCountWithoutIph)
{
    csdw_.IPH = 0;
    ValidateSubpacketCount(packet_header_, csdw_, context_, component_);
}