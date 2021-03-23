#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "binbuff.h"
#include "ch10_packet.h"
#include "ch10_videof0_component.h"
#include "ch10_videof0_test_data.h"

/* Helper function Prototypes *****/
void ValidateSubpacketCount(
    Ch10PacketHeaderFmt packet_header, 
    Ch10VideoF0HeaderFormat csdw, 
    Ch10Context &context, 
    Ch10VideoF0Component component);

// std::vector<uint8_t> GetVideoData(
//     Ch10Context &context, Ch10VideoF0HeaderFormat csdw, uint32_t subpacket_count, uint32_t rtc_offset=100);

// template <typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
// void AppendBytes(std::vector<uint8_t> &v, T value);
/**********************************/

/* Mocks **************************/
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

// class BinBuffMock : public BinBuff
// {
// public:
//     BinBuffMock(std::vector<uint8_t> data) : BinBuff()
//     {
//         bytes_ = data;
//         is_initialized_ = true;
//         read_count_ = bytes_.size();
//     }
// };
/**********************************/


class Ch10VideoF0ComponentTest : public ::testing::Test
{
protected:
    std::vector<uint8_t> test_data_;
    // BinBuffMock bb_;
    Ch10VideoF0ComponentMock component_;
    Ch10PacketHeaderFmt packet_header_;
    Ch10Context context_;
    Ch10VideoF0HeaderFormat csdw_;
    Ch10PacketElement<Ch10VideoF0HeaderFormat> csdw_element;

    uint8_t no_iph_data_[4+16*188];
    uint8_t iph_data_[4*16*(8+188)];
 
    const uint32_t TDP_RTC1 = 3210500;
    const uint32_t TDP_RTC2 = 502976;
    const uint64_t TDP_ABSOLUTE_TIME = 344199919;
    const uint8_t TDP_DAY_OF_YEAR = 0;

    Ch10VideoF0ComponentTest() : 
    test_data_(Ch10VideoF0TestData::data), component_(&context_)//, bb_(test_data_)
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
        absolute_position = 4823829394;
        context_.UpdateContext(absolute_position, &packet_header_);

        csdw_.BA = 0;
        csdw_.PL = 1;
        csdw_.KLV = 1;
        csdw_.SRS = 1;
        csdw_.IPH = 1;
        csdw_.ET = 1;

        // Ch10VideoF0HeaderFormat *icp=(Ch10VideoF0HeaderFormat*)iph_data_;
        // Ch10VideoF0HeaderFormat *ncp=(Ch10VideoF0HeaderFormat*)no_iph_data_;
        // *ncp=csdw_;
        // /*****/for(int i=0; i<sizeof(csdw_); i++) 
        // /*****/{
        // /*****/    printf("id[%d]=%X\n", i, iph_data_[i]);
        // /*****/    printf("nd[%d]=%X\n", i, no_iph_data_[i]);
        // /*****/}
        // ncp->IPH = 0;
        // int *rp;
        // char bogus[]="bogus";
        // for(int i=0;i<16;i++)
        // {
        //     uint32_t *rp;
        //     rp = (uint32_t*)(void*)(iph_data_+i*(8+188));
        //     *rp=TDP_RTC1 + 16*(i+2);
        //     *(rp+1)=TDP_RTC2;
        //     for(int j=0; j<188; j++)
        //     {
        //         uint8_t *ip=iph_data_+4+(8+i*188);
        //         uint8_t *np=iph_data_+4+(i*188);
        //         *np=(uint8_t)bogus[i%5];
        //         *ip=*np;
        //     }
        // }

        // context_.Initialize(0, 99999);
        // context_.SetSearchingForTDP(false);
        // context_.UpdateWithTDPData(1'000'000, 1, true);
        // std::map<Ch10PacketType, bool> type_config_map = {{Ch10PacketType::VIDEO_DATA_F0, true}};
        // context_.SetPacketTypeConfig(type_config_map);
        // std::vector<std::string> dummy_tmats_strings;
        // Ch10Packet packet(&bb_, &context_, dummy_tmats_strings);
        // packet.ParseHeader();
    }
};

// TEST_F(Ch10VideoF0ComponentTest, data)
// {
//     uint8_t *cp = (uint8_t*)&csdw_;
//     printf("%d =? %d\n", *cp, iph_data_[0]);
//     // EXPECT_EQ(cp[0], iph_data_[0]);
//     // EXPECT_EQ(cp[1], iph_data_[1]);
//     // EXPECT_EQ(cp[2], iph_data_[2]);
//     // EXPECT_EQ(cp[3], iph_data_[3]);

//     // EXPECT_EQ(cp[0], no_iph_data_[0]);
//     // EXPECT_EQ(cp[1], no_iph_data_[1]);
//     // EXPECT_EQ(cp[2], no_iph_data_[2]);
//     // EXPECT_NE(cp[3], no_iph_data_[3]);

//     Ch10VideoF0HeaderFormat *ncp = (Ch10VideoF0HeaderFormat*)no_iph_data_;
//     EXPECT_EQ(0, ncp->IPH);
// }

TEST_F(Ch10VideoF0ComponentTest, GetSubpacketCount)
{
    ASSERT_EQ(188, component_.GetSubPacketCount(false));
    ASSERT_EQ(188+context_.intrapacket_ts_size_, component_.GetSubPacketSize(true));
}

TEST_F(Ch10VideoF0ComponentTest, ParseAvancesDataPointer)
{
    Ch10Status status;
    const uint8_t* data_ptr = (uint8_t*)&csdw_;
    // const uint8_t* data_ptr = bb_.Data();
    const uint8_t* expected = data_ptr + sizeof(csdw_);

    /********/printf("Calling Parse\n");
    status = component_.Parse(data_ptr);
    /********/printf("Done with Parse\n");

    EXPECT_EQ(data_ptr, expected);
}

// TEST_F(Ch10VideoF0ComponentTest, ParseCreatesCsdwElement)
// {
//     Ch10Status status;
//     uint8_t* data_ptr = (uint8_t*)&csdw_;
//     uint8_t* expected = data_ptr;

//     status = component_.Parse((const uint8_t* &)data_ptr);

//     // EXPECT_EQ((uint8_t*)component_.csdw_element.element, expected);
//     EXPECT_EQ((*component_.csdw_element.element)->BA, csdw_.BA);
//     EXPECT_EQ((*component_.csdw_element.element)->PL, csdw_.PL);
//     EXPECT_EQ((*component_.csdw_element.element)->KLV, csdw_.KLV);
//     EXPECT_EQ((*component_.csdw_element.element)->SRS, csdw_.SRS);
//     EXPECT_EQ((*component_.csdw_element.element)->IPH, csdw_.IPH);
//     EXPECT_EQ((*component_.csdw_element.element)->ET, csdw_.ET);
// }

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
    packet_header_.data_size = 1 * TransportStream_UNIT_SIZE;
    context_.UpdateContext(0, &packet_header_);
    uint64_t expected_absolute_time = context_.tdp_abs_time + (context_.rtc) - context_.tdp_rtc;

    csdw_.IPH = 0;
    uint8_t* data_ptr = (uint8_t*)&csdw_;
    component_.Parse((const uint8_t* &)data_ptr);
    
    std::vector<uint64_t> times =  component_.GetTimes();
    EXPECT_EQ(expected_absolute_time, times[0]);
}

// TEST_F(Ch10VideoF0ComponentTest, ParseSetsTimesFromIPHeaders)
// {
//     csdw_.IPH = 1;
//     uint8_t subpacket_size = TransportStream_UNIT_SIZE + context_.intrapacket_ts_size_;
//     uint8_t subpacket_count = 13;
//     packet_header_.data_size = subpacket_count * subpacket_size;
//     context_.UpdateContext(0, &packet_header_);

//     uint32_t rtc_offset = 123;
//     std::vector<uint8_t> data_vector = GetVideoData(context_, csdw_, subpacket_count, rtc_offset);
// /******/return;
//     std::vector<uint64_t> expected_absolute_times(subpacket_count);
//     uint64_t packet_absolute_time = context_.CalculateAbsTimeFromRTCNanoseconds(context_.rtc);
//     for (size_t i=0; i < subpacket_count; i++)
//     {
//         expected_absolute_times[i] = packet_absolute_time + (i+1) * rtc_offset;
//     }

//     uint8_t* data_ptr = (uint8_t*)data_vector.data();
//     mock_component_.Parse((const uint8_t* &)data_ptr);
    
//     std::vector<uint64_t> actual_times =  mock_component_.GetTimes();
//     EXPECT_EQ(expected_absolute_times, actual_times);

//     printf("Nothing\n");
//     printf("Nothing\n");
//     actual_times[0] = 0;
//     EXPECT_NE(expected_absolute_times, actual_times);
//     printf("Nothing more\n");
//     printf("Nothing more\n");
//     printf("Nothing more\n");
//     printf("Nothing more\n");
// }

/* Helper functions **********************/
// std::vector<uint8_t> GetVideoData(
//     Ch10Context &context, Ch10VideoF0HeaderFormat csdw, uint32_t subpacket_count, uint32_t rtc_offset)
// {
//     size_t subpacket_size = TransportStream_UNIT_SIZE;
//     if(csdw.IPH) subpacket_size += context.intrapacket_ts_size_;
    
//     char bogus[] = {'b', 'o', 'g', 'u', 's' };
//     uint32_t rtc1 = (uint32_t)(context.rtc / 100); // convert back from nanoseconds to original RTC
//     uint32_t rtc2 = (uint32_t)((context.rtc / 100) >> 32);
//     std::vector<uint8_t> data_vector;

//     AppendBytes<uint32_t>(data_vector, *(uint32_t*)&csdw);
//     /*****/printf("CSDW: %08X, dv[]: %02X%02X%02X%02X\n", csdw, 
//     /*****/    data_vector[data_vector.size()-4], 
//     /*****/    data_vector[data_vector.size()-3],
//     /*****/    data_vector[data_vector.size()-2], 
//     /*****/    data_vector[data_vector.size()-1]); 
//     /*****/for (uint8_t b : data_vector)
//     /*****/{
//     /*****/    printf("%02X ", b);
//     /*****/}
//     /*****/printf("\n");

//     // Fill with bogus data

//     for (uint32_t packet=0; packet < subpacket_count; packet++)
//     {
//         if (csdw.IPH) 
//         {
//             rtc1 += rtc_offset;
//             AppendBytes<uint32_t>(data_vector, rtc1);
//             /*****/printf("rtc1: %08X, dv[]: %02X%02X%02X%02X\n", rtc1, 
//             /*****/    data_vector[data_vector.size()-4], 
//             /*****/    data_vector[data_vector.size()-3],
//             /*****/    data_vector[data_vector.size()-2], 
//             /*****/    data_vector[data_vector.size()-1]); 
//     /*****/if(packet==0)for (uint8_t b : data_vector)
//     /*****/{
//     /*****/    printf("%02X ", b);
//     /*****/}
//     /*****/printf("\n");

//             AppendBytes<uint32_t>(data_vector, rtc2);
//             /*****/printf("rtc2: %08X, dv[]: %02X%02X%02X%02X\n", rtc2, 
//             /*****/    data_vector[data_vector.size()-4], 
//             /*****/    data_vector[data_vector.size()-3],
//             /*****/    data_vector[data_vector.size()-2], 
//             /*****/    data_vector[data_vector.size()-1]); 
//     /*****/if(packet==0)for (uint8_t b : data_vector)
//     /*****/{
//     /*****/    printf("%02X ", b);
//     /*****/}
//     /*****/printf("\n");
//         }

//         for (size_t j=0; j < TransportStream_UNIT_SIZE; j++)
//         {
//             size_t i = j % sizeof(bogus);
//             data_vector.push_back( bogus[i] );
//             /*****/printf("%c", data_vector[data_vector.size()-1]);
//         }
//         /*****/printf("\n");
//     }

//     return data_vector;
// }


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

// /*
//     Appends the bytes contained in a number value to the end of a vector of bytes.
//     The least significant byte is appended first.  
//     Inputs:
//         v     -> A vector of bytes to append to
//         value -> A number whose bytes will be appended to v
//     Example:
//         Given: v = {0x01, 0x02, 0x03, 0x04} and value = 0xAABBCCDD
//         Call:
//                 AppendBytes(v, value);
//         Result: v = {0x01, 0x02, 0x03, 0x04, 0xDD, 0xCC, 0xBB, 0xAA}
// */
// template <typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type*>
// void AppendBytes(std::vector<uint8_t> &v, T value)
// {
//     uint64_t mask = 0xFF;
//     /********/printf("Starting: %016lX\n", value);
//     for (int8_t i = sizeof(T)-1; i >= 0; i--)
//     {
//         /********/ printf("Processing %016lX\n", value);
//         v.push_back( value & mask );
//         /********/ printf("%02X added. ", value & mask);
//         value >>= 8;
//         /********/ printf("New value %016lX\n", value);
//     }
// }


/* Helper function tests *****************/
// TEST_F(Ch10VideoF0ComponentTest, AppendWorksWithVariousTypes)
// {
//     std::vector<uint8_t> vector;
//     size_t expected_size = 0;
    
//     AppendBytes(vector, (uint8_t) 0xAA);
//     AppendBytes(vector, (uint16_t)0xAABB);
//     AppendBytes(vector, (uint32_t)0xAABBCCDD);
//     AppendBytes(vector, (uint64_t)0xAABBCCDDEEFF0011);

//     std::vector<uint8_t> expected{ 
//         0xAA,                   // 0xAA
//         0xBB, 0xAA,             // 0xAABB
//         0xDD, 0xCC, 0xBB, 0xAA, // 0xAABBCCDD
//         0x11, 0x00, 0xFF, 0xEE, 
//         0xDD, 0xCC, 0xBB, 0xAA  // 0xAABBCCDDEEFF0011
//     };
//     ASSERT_EQ(expected, vector);
// }

// TEST_F(Ch10VideoF0ComponentTest, GetVideoData)
// {
//     csdw_.IPH = 1;
//     uint32_t subpacket_count = 5;
//     uint32_t rtc_offset = 16;
//     std::vector<uint8_t>  data_vector = 
//         GetVideoData(context_, csdw_, subpacket_count, rtc_offset);


//     size_t expected_size =
//         sizeof(csdw_) + subpacket_count * (context_.intrapacket_ts_size_ + TransportStream_UNIT_SIZE);
//     ASSERT_EQ(expected_size, data_vector.size());

//     // /*****/printf("CSDW: %08X, dv[]: %02X%02X%02X%02X\n",
//     // /*****/    csdw_, 
//     // /*****/    data_vector[0], 
//     // /*****/    data_vector[1], 
//     // /*****/    data_vector[2], 
//     // /*****/    data_vector[3]);

//     uint8_t *data = data_vector.data();
//     uint8_t *csdw_ptr = (uint8_t*)&csdw_;
//     for (int i = 0; i < sizeof(csdw_); i++)
//     {
//         EXPECT_EQ(*csdw_ptr, *data);
//     }

//     data += sizeof(csdw_);
//     uint32_t expected_rtc1 = packet_header_.rtc1 + rtc_offset;
//     uint32_t expected_rtc2 = packet_header_.rtc2;
//     for (int packet_no=0; packet_no < subpacket_count; packet_no++)
//     {
//         expected_rtc1 += rtc_offset;
//         uint32_t *rtc_pointer = (uint32_t *)data;
//         ASSERT_EQ(expected_rtc1, *rtc_pointer);
//         ASSERT_EQ(expected_rtc2, *(rtc_pointer+1));
        
//         data += sizeof(uint32_t) * 2;

//         for (int j = 0; j < subpacket_count; j++)
//         {
//             video_datum *video_datum_ptr = (video_datum*)data;
//             ASSERT_EQ(packet_no * j, *video_datum_ptr);

//             data += sizeof(video_datum);
//         }
//     }
// }