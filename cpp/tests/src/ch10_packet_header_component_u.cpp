#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_packet_header_component.h"

class Ch10PacketHeaderComponentTest : public ::testing::Test
{
protected:
    Ch10PacketHeaderFmt pkt_hdr_fmt_;
    Ch10PacketHeaderComponent ch10_pkt_hdr_comp_;
    uint64_t loc_;
    const uint8_t* data_ptr_;
    const uint8_t* orig_data_ptr_;
    Ch10Status status_;

    Ch10PacketHeaderComponentTest() : loc_(0), data_ptr_(nullptr), orig_data_ptr_(nullptr),
        status_(Ch10Status::NONE)
    {
        // Fill out values for fake header data.
        pkt_hdr_fmt_.sync = ch10_pkt_hdr_comp_.sync_;
        pkt_hdr_fmt_.chanID = 1;
        pkt_hdr_fmt_.pkt_size = 1000;
        pkt_hdr_fmt_.data_size = 800;
        pkt_hdr_fmt_.data_type_ver = 1;
        pkt_hdr_fmt_.seq_num = 0;
        pkt_hdr_fmt_.checksum_existence = 2; // 16-bit header checksum
        pkt_hdr_fmt_.time_format = 0;
        pkt_hdr_fmt_.overflow_err = 0;
        pkt_hdr_fmt_.sync_err = 0;
        pkt_hdr_fmt_.intrapkt_ts_source = 0;
        pkt_hdr_fmt_.secondary_hdr = 0;
        pkt_hdr_fmt_.data_type = 0x01; // TMATS
        pkt_hdr_fmt_.rtc1 = 0; // set later
        pkt_hdr_fmt_.rtc2 = 0; // set later
        pkt_hdr_fmt_.checksum = 0; // set later
    }
};

TEST_F(Ch10PacketHeaderComponentTest, ParseGoodSync)
{
    // Parse pre-defined fake header and check data.
    data_ptr_ = (const uint8_t*)&pkt_hdr_fmt_;
    orig_data_ptr_ = data_ptr_;
    status_ = ch10_pkt_hdr_comp_.Parse(data_ptr_, loc_);

    // Check a few elements.
    EXPECT_EQ((*ch10_pkt_hdr_comp_.std_hdr_elem.element)->chanID, 1);
    EXPECT_EQ((*ch10_pkt_hdr_comp_.std_hdr_elem.element)->data_size, 800);
    EXPECT_EQ((*ch10_pkt_hdr_comp_.std_hdr_elem.element)->data_type, 0x01);

    // Check that loc_ and pointer have been incremented.
    EXPECT_EQ(loc_, ch10_pkt_hdr_comp_.std_hdr_elem.size);
    EXPECT_EQ(data_ptr_, orig_data_ptr_ + ch10_pkt_hdr_comp_.std_hdr_elem.size);

    EXPECT_EQ(status_, Ch10Status::OK);
}

TEST_F(Ch10PacketHeaderComponentTest, ParseBadSync)
{
    // Set bad sync value.
    pkt_hdr_fmt_.sync = 10;

    // Parse pre-defined fake header and check data.
    data_ptr_ = (const uint8_t*)&pkt_hdr_fmt_;
    status_ = ch10_pkt_hdr_comp_.Parse(data_ptr_, loc_);

    // There is no need to check that the raw bits are parsed
    // correctly, as is done in ParseGoodSync since the basic
    // functionality of ParseElements is tested in ch10_packet_component_u.cpp.

    EXPECT_EQ(status_, Ch10Status::BAD_SYNC);
}

TEST_F(Ch10PacketHeaderComponentTest, ParseSecondaryHeaderNotPresent)
{
    // Set secondary header not present.
    pkt_hdr_fmt_.secondary_hdr = 0;

    // Parse pre-defined fake header and check data.
    data_ptr_ = (const uint8_t*)&pkt_hdr_fmt_;
    orig_data_ptr_ = data_ptr_;
    status_ = ch10_pkt_hdr_comp_.Parse(data_ptr_, loc_);
    EXPECT_EQ(status_, Ch10Status::OK);

    status_ = ch10_pkt_hdr_comp_.ParseSecondaryHeader(data_ptr_, loc_);
    EXPECT_EQ(status_, Ch10Status::OK);
}

TEST_F(Ch10PacketHeaderComponentTest, ParseSecondaryHeaderInvalidFormat)
{
    // Set secondary header not present.
    pkt_hdr_fmt_.secondary_hdr = 1;
    pkt_hdr_fmt_.time_format = 3; // invalid

    // Parse pre-defined fake header and check data.
    data_ptr_ = (const uint8_t*)&pkt_hdr_fmt_;
    orig_data_ptr_ = data_ptr_;
    status_ = ch10_pkt_hdr_comp_.Parse(data_ptr_, loc_);
    EXPECT_EQ(status_, Ch10Status::OK);

    status_ = ch10_pkt_hdr_comp_.ParseSecondaryHeader(data_ptr_, loc_);
    EXPECT_EQ(status_, Ch10Status::INVALID_SECONDARY_HDR_FMT);
}