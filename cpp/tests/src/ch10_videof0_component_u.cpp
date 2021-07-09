#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_videof0_component.h"
#include "ch10_time.h"

/***** Mock Component *****/
/***** class for inspecting class variables *****/
class Ch10VideoF0ComponentMock : public Ch10VideoF0Component
{
    // Inherits subpacket_absolute_times_
   public:
    Ch10VideoF0ComponentMock(Ch10Context *context_ptr) : Ch10VideoF0Component(context_ptr) {}

    std::vector<uint64_t> GetTimes()
    {
        return subpacket_absolute_times_;
    }

    const video_datum *GetPointerFromVideoElement()
    {
        Ch10PacketElement<video_datum> *element = (Ch10PacketElement<video_datum> *)video_element_vector_[0];
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
    uint64_t rtc_;
    Ch10Time ch10_time_;
    Ch10Status status_;

    const uint32_t TDP_RTC1 = 3210500;
    const uint32_t TDP_RTC2 = 502976;
    const uint64_t TDP_ABSOLUTE_TIME = 344199919;
    const uint8_t TDP_DAY_OF_YEAR = 0;

    Ch10VideoF0ComponentTest() : component_(&context_), rtc_(0), ch10_time_()
    {
        // Set up packet as if with a TDP packet
        packet_header_.rtc1 = TDP_RTC1;
        packet_header_.rtc2 = TDP_RTC2;
        packet_header_.intrapkt_ts_source = 0;
        context_.UpdateContext(0, &packet_header_,
                               ch10_time_.CalculateRTCTimeFromComponents(packet_header_.rtc1, packet_header_.rtc2));
        context_.UpdateWithTDPData(TDP_ABSOLUTE_TIME, TDP_DAY_OF_YEAR, true);

        // Now update context as if with a new non-TDP packet
        packet_header_.rtc1 = TDP_RTC1 + 100;
        packet_header_.rtc2 = TDP_RTC2;
        packet_header_.chanID = 4;
        context_.UpdateContext(0, &packet_header_,
                               ch10_time_.CalculateRTCTimeFromComponents(packet_header_.rtc1, packet_header_.rtc2));

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
    EXPECT_EQ(188 + context_.intrapacket_ts_size_, component_.GetSubpacketSize(true));
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
    uint8_t subpacket_size = TransportStream_UNIT_SIZE;
    subpacket_size += context_.intrapacket_ts_size_;

    // Test integer (complete) packet count
    packet_header_.data_size = 5 * subpacket_size - 1;
    context_.UpdateContext(0, &packet_header_, rtc_);

    const uint8_t *data_ptr = (uint8_t *)&csdw_;
    status_ = component_.Parse(data_ptr);
    EXPECT_EQ(Ch10Status::VIDEOF0_NONINTEGER_SUBPKT_COUNT, status_);
}

TEST_F(Ch10VideoF0ComponentTest, ParseRejectsNonintegerSubpacketCountNoIPH)
{
    csdw_.IPH = 0;
    uint8_t subpacket_size = TransportStream_UNIT_SIZE;

    // Test integer (complete) packet count
    packet_header_.data_size = 5 * subpacket_size - 1;
    context_.UpdateContext(0, &packet_header_, rtc_);

    const uint8_t *data_ptr = (uint8_t *)&csdw_;
    status_ = component_.Parse(data_ptr);
    EXPECT_EQ(Ch10Status::VIDEOF0_NONINTEGER_SUBPKT_COUNT, status_);
}

TEST_F(Ch10VideoF0ComponentTest, ParseSubpacketNoIPHSetsTimeToPacketTime)
{
    uint64_t current_rtc = ((uint64_t(packet_header_.rtc2) << 32) + uint64_t(packet_header_.rtc1)) * 100;
    uint64_t packet_time = context_.CalculateAbsTimeFromRTCFormat(current_rtc);

    // Call ParseSubpacket with a bogus pointer
    // It shouldn't attempt to read or increment the pointer if we pass false for IPH
    uint8_t dummy;
    const uint8_t *data = (uint8_t *)&dummy;
    status_ = component_.ParseSubpacket(data, false, 0);
    EXPECT_EQ(Ch10Status::OK, status_);

    std::vector<uint64_t> times = component_.GetTimes();
    ASSERT_TRUE(times.size() > 0);
    ASSERT_EQ(packet_time, times[0]);
    ;
}

TEST_F(Ch10VideoF0ComponentTest, ParseSubpacketIPHSetsTimeToSubpacketTime)
{
    uint64_t current_rtc = ((uint64_t(packet_header_.rtc2) << 32) + uint64_t(packet_header_.rtc1)) * 100;
    uint64_t packet_time = context_.CalculateAbsTimeFromRTCFormat(current_rtc);

    Ch10RTCTimeStampFmt rtc;
    rtc.ts1_ = packet_header_.rtc1 + 1;  // Increment least-significant word (first word in VideoF0 timestamp)
    rtc.ts2_ = packet_header_.rtc2;

    const uint8_t *data = (const uint8_t *)&rtc;
    status_ = component_.ParseSubpacket(data, true, 0);
    EXPECT_EQ(Ch10Status::OK, status_);

    std::vector<uint64_t> times = component_.GetTimes();
    ASSERT_TRUE(times.size() > 0);
    ASSERT_EQ(packet_time + 100, times[0]);
}

TEST_F(Ch10VideoF0ComponentTest, ParseSubpacketIPHParsesPayload)
{
    // Create a block of memory with 8 bytes of IPH time stamp and
    // 188 bytes of video data. Set the first 8 bytes to zero to
    // facilitate comparison later.
    std::vector<video_datum> original_vector(TransportStream_DATA_COUNT +
                                                 8 / RECORDED_DATA_SIZE,
                                             54321);
    original_vector[0] = 0;
    original_vector[1] = 0;
    original_vector[2] = 0;
    original_vector[3] = 0;
    const uint8_t *data = (const uint8_t *)original_vector.data();

    status_ = component_.ParseSubpacket(data, true, 0);
    EXPECT_EQ(Ch10Status::OK, status_);

    // Increment pointer by IPH size plus TS data size
    uint8_t *expected_next_ptr = (uint8_t *)original_vector.data();
    expected_next_ptr += (8 + TransportStream_UNIT_SIZE);
    ASSERT_EQ(expected_next_ptr, data);

    const video_datum *video_data = component_.GetPointerFromVideoElement();

    EXPECT_EQ(original_vector.data() + 8 / RECORDED_DATA_SIZE, video_data);
    for (int i = 0; i < TransportStream_DATA_COUNT; i++)
    {
        EXPECT_EQ(54321, *(video_data + i));
    }
}

TEST_F(Ch10VideoF0ComponentTest, ParseSubpacketNoIPHParsesPayload)
{
    std::vector<video_datum> original_vector(TransportStream_DATA_COUNT, 54321);
    const uint8_t *data = (const uint8_t *)original_vector.data();

    status_ = component_.ParseSubpacket(data, false, 0);
    EXPECT_EQ(Ch10Status::OK, status_);

    // Increment pointer by TS data size
    uint8_t *expected_next_ptr = (uint8_t *)original_vector.data() + TransportStream_UNIT_SIZE;
    EXPECT_EQ(expected_next_ptr, data);

    const video_datum *video_data = component_.GetPointerFromVideoElement();
    EXPECT_EQ(original_vector.data(), video_data);
    for (int i = 0; i < TransportStream_DATA_COUNT; i++)
    {
        EXPECT_EQ(54321, *(video_data + i));
    }
}
