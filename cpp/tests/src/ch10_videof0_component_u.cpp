#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_videof0_component.h"

class Ch10VideoF0ComponentMock : public Ch10VideoF0Component
{
    // Inherits subpacket_absolute_times_
public:
    Ch10VideoF0ComponentMock(Ch10Context* context_ptr) : Ch10VideoF0Component(context_ptr) {}
    std::vector<uint64_t> GetTimes()
    {
        return subpacket_absolute_times_;
    }
};

class Ch10VideoF0ComponentTest : public ::testing::Test
{
protected:
    Ch10VideoF0Component component_;
    Ch10VideoF0ComponentMock mock_component_;
    Ch10PacketHeaderFmt packet_header_;
    Ch10Context context_;
    Ch10VideoF0HeaderFormat csdw_;
    Ch10PacketElement<Ch10VideoF0HeaderFormat> csdw_element;

    const uint64_t TDP_RTC1 = 3210500;
    const uint64_t TDP_RTC2 = 502976;
    const uint64_t TDP_ABSOLUTE_TIME = 344199919;
    const uint8_t TDP_DAY_OF_YEAR = 0;

    Ch10VideoF0ComponentTest() : mock_component_(&context_), component_(&context_)
    {
        // Set up packet rtc as if this is the TDP packet
        packet_header_.rtc1 = TDP_RTC1;
        packet_header_.rtc2 = TDP_RTC2;
        uint64_t absolute_position = 4823829000;
        context_.UpdateContext(absolute_position, &packet_header_);

        // Update context with TDP-specific data.
        context_.UpdateWithTDPData(TDP_ABSOLUTE_TIME, TDP_DAY_OF_YEAR, true);

        // Update context with "current" header data.
        packet_header_.rtc1 = TDP_RTC1 + 100;
        packet_header_.rtc2 = TDP_RTC2;
        packet_header_.chanID = 4;
        packet_header_.data_size = 55;
        packet_header_.pkt_size = 80;
        absolute_position = 4823829394;
        context_.UpdateContext(absolute_position, &packet_header_);

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

TEST_F(Ch10VideoF0ComponentTest, ParseWithoutIPHSetsFirstTimeToPacketTime)
{
    printf("Starting ParseWithoutIPHSetsFirstTimeToPacketTime\n");
    packet_header_.data_size = 0;
    context_.UpdateContext(0, &packet_header_);
    uint64_t expected_absolute_time = context_.CalculateAbsTimeFromRTCNanoseconds(context_.rtc);
    expected_absolute_time = TDP_ABSOLUTE_TIME + (context_.rtc) - context_.tdp_rtc;

    csdw_.IPH = 0;
    uint8_t* data_ptr = (uint8_t*)&csdw_;
    printf("Calling mock_component_Parse\n");
    mock_component_.Parse((const uint8_t* &)data_ptr);
    printf("Finished call to Parse\n");
    
    // std::vector<uint64_t> times =  mock_component_.GetTimes();
    // printf("times.size: %d", times.size());
    // EXPECT_EQ(expected_absolute_time, times[0]);

    // for (size_t i=1; i<times.size(); i++)
    // {
    //     EXPECT_EQ(0, times[0]);
    // }
}