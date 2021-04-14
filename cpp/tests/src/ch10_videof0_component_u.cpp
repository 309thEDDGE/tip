#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_videof0_component.h"


/***** Mock Component *****/
/***** class for inspecting class variables *****/
class Ch10VideoF0ComponentMock : public Ch10VideoF0Component
{
    // Inherits subpacket_absolute_times_
public:
    Ch10VideoF0ComponentMock(Ch10Context* context_ptr) : Ch10VideoF0Component(context_ptr) {}

    std::vector<uint64_t> GetTimes()
    {
        return subpacket_absolute_times_;
    }

    // std::vector<video_datum> GetVideoData()
    // {
    //     /***/printf("Here goes! Creating video data vector");
    //     std::vector<video_datum> video_data;
    //     for (Ch10PacketElementBase *base : video_datum_elements_vector_)
    //     {
    //         Ch10PacketElement<video_datum> *element = (Ch10PacketElement<video_datum> *)base;
    //         video_data.push_back( **(element->element) );
    //     }
    //     return video_data;
    // }

    const video_datum* GetPointerFromVideoElement()
    {
        Ch10PacketElement<video_datum> *element = (Ch10PacketElement<video_datum>*) video_element_vector_[0];
        return *element->element;
    }
};

/**** Test Class *****/
class Ch10VideoF0ComponentTest : public ::testing::Test
{
protected:
    Ch10VideoF0ComponentMock component_;
    Ch10PacketHeaderFmt packet_header_;
    Ch10Context context_;
    Ch10VideoF0HeaderFormat csdw_;

    const uint32_t TDP_RTC1 = 3210500;
    const uint32_t TDP_RTC2 = 502976;
    const uint64_t TDP_ABSOLUTE_TIME = 344199919;
    const uint8_t TDP_DAY_OF_YEAR = 0;

    Ch10VideoF0ComponentTest() : component_(&context_)
    {
        // Set up packet as if with a TDP packet
        packet_header_.rtc1 = TDP_RTC1;
        packet_header_.rtc2 = TDP_RTC2;
        context_.UpdateContext(0, &packet_header_);
        context_.UpdateWithTDPData(TDP_ABSOLUTE_TIME, TDP_DAY_OF_YEAR, true);

        // Now update context as if with a new non-TDP packet
        packet_header_.rtc1 = TDP_RTC1 + 100;
        packet_header_.rtc2 = TDP_RTC2;
        packet_header_.chanID = 4;
        context_.UpdateContext(0, &packet_header_);

        csdw_.BA = 0;
        csdw_.PL = 1;
        csdw_.KLV = 1;
        csdw_.SRS = 1;
        csdw_.IPH = 1;
        csdw_.ET = 1;
    }
};

/***** Test Methods *****/
TEST_F(Ch10VideoF0ComponentTest, GetSubpacketSize)
{
    EXPECT_EQ(188, component_.GetSubpacketSize(false));
    EXPECT_EQ(188+context_.intrapacket_ts_size_, component_.GetSubpacketSize(true));
}

TEST_F(Ch10VideoF0ComponentTest, DivideExactIntegerReturnsNegativeOnNonintegerResult)
{
    int32_t count = component_.DivideExactInteger(11, 5);
    EXPECT_TRUE(count < 0);
}

TEST_F(Ch10VideoF0ComponentTest, DivideExactIntegerReturnsCorrectIntegerResult)
{
    int32_t count = component_.DivideExactInteger(10, 5);
    EXPECT_EQ(count, 2);
}

TEST_F(Ch10VideoF0ComponentTest, ParseRejectsNonintegerSubpacketCountIPH)
{
    csdw_.IPH = 1;
    Ch10Status status;
    uint8_t subpacket_size = TransportStream_UNIT_SIZE;
    subpacket_size += context_.intrapacket_ts_size_;

    // Test integer (complete) packet count
    packet_header_.data_size = 5 * subpacket_size - 1;
    context_.UpdateContext(0, &packet_header_);

    const uint8_t* data_ptr = (uint8_t *)&csdw_;
    status = component_.Parse(data_ptr);
    EXPECT_EQ(Ch10Status::VIDEOF0_NONINTEGER_SUBPKT_COUNT, status);
}

TEST_F(Ch10VideoF0ComponentTest, ParseRejectsNonintegerSubpacketCountNoIPH)
{
    csdw_.IPH = 0;
    Ch10Status status;
    uint8_t subpacket_size = TransportStream_UNIT_SIZE;

    // Test integer (complete) packet count
    packet_header_.data_size = 5 * subpacket_size - 1;
    context_.UpdateContext(0, &packet_header_);

    const uint8_t* data_ptr = (uint8_t *)&csdw_;
    status = component_.Parse(data_ptr);
    EXPECT_EQ(Ch10Status::VIDEOF0_NONINTEGER_SUBPKT_COUNT, status);
}

TEST_F(Ch10VideoF0ComponentTest, ParseSubpacketTimeNoIPHReturnsPacketTime)
{
    // Set up pointers to dummy data to compare before and after
    uint8_t dummy;
    uint8_t *p_original = &dummy;
    const uint8_t *p_data = &dummy;

    // Get packet time for comparison
    uint64_t packet_time = context_.GetPacketAbsoluteTime();

    // Get subpacket time with no intrapacket header
    uint64_t subpacket_time = component_.ParseSubpacketTime(p_data, false);

    // Subpacket time should default to the packet time
    EXPECT_EQ(packet_time, subpacket_time);

    // Data pointer should stay the same, not advanced
    EXPECT_EQ(p_original, p_data);
}

TEST_F(Ch10VideoF0ComponentTest, ParseSubpacketTimeIPHReturnsSubpacketTime)
{
    Ch10VideoF0RTCTimeStampFmt time_stamp{ packet_header_.rtc1 + 1, packet_header_.rtc2 };
    const uint8_t *data = (uint8_t *)&time_stamp;
    uint8_t *expected_ptr = (uint8_t *)(&time_stamp + 1);

    uint64_t expected_time = context_.CalculateAbsTimeFromRTCFormat(time_stamp.ts1_, time_stamp.ts2_);

    // Parse the time with IPH=true so that the time stamp will be used
    //    to calculate the subpacket absolute time
    uint64_t subpacket_absolute_time = component_.ParseSubpacketTime(data, true);
    EXPECT_EQ(expected_time, subpacket_absolute_time);    
    
    // Make sure data pointer was advanced past the timestamp
    EXPECT_EQ(expected_ptr, data);
}

TEST_F(Ch10VideoF0ComponentTest, ParseSubpacketNoIPHSetsTimeToPacketTime)
{
    uint64_t packet_time = context_.CalculateAbsTimeFromRTCFormat(packet_header_.rtc1, packet_header_.rtc2);

    // Call ParseSubpacket with a bogus pointer
    // It shouldn't attempt to read or increment the pointer if we pass false for IPH
    uint8_t dummy;
    const uint8_t *data = (uint8_t*)&dummy;
    component_.ParseSubpacket(data, false);
    
    std::vector<uint64_t> times = component_.GetTimes();
    ASSERT_TRUE(times.size() > 0);
    ASSERT_EQ(packet_time, times[0]);;
}

TEST_F(Ch10VideoF0ComponentTest, ParseSubpacketIPHSetsTimeToSubpacketTime)
{
    uint64_t packet_time = context_.CalculateAbsTimeFromRTCFormat(packet_header_.rtc1, packet_header_.rtc2);

    Ch10VideoF0RTCTimeStampFmt  rtc;
    rtc.ts1_ = packet_header_.rtc1 + 1; // Increment least-significant word (first word in VideoF0 timestamp)
    rtc.ts2_ = packet_header_.rtc2;

    const uint8_t *data = (uint8_t*)&rtc;
    component_.ParseSubpacket(data, true);

    std::vector<uint64_t> times = component_.GetTimes();
    ASSERT_TRUE(times.size() > 0);
    ASSERT_EQ(packet_time + 100, times[0]);
}

TEST_F(Ch10VideoF0ComponentTest, ParseSubpacketNoIPHParsesPayload)
{
    std::vector<video_datum> original_vector(TransportStream_DATA_COUNT, 54321);
    const uint8_t *data = (uint8_t*)original_vector.data();

    component_.ParseSubpacket(data, false);
    uint8_t *expected_next_ptr = (uint8_t*)(original_vector.data() + TransportStream_DATA_COUNT);
    ASSERT_EQ(expected_next_ptr, data);

    const video_datum *video_data = component_.GetPointerFromVideoElement();
    ASSERT_EQ(original_vector.data(), video_data);
    for (int i = 0; i < TransportStream_DATA_COUNT; i++)
    {
        ASSERT_EQ(54321, *(video_data + i));
    }
}
