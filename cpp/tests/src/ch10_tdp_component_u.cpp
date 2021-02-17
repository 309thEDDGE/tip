#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_tdp_component.h"

class Ch10TDPComponentTest : public ::testing::Test
{
protected:
    TDF1CSDWFmt tdp_fmt_;
    TDF1DataIRIGFmt irig_fmt_;
    TDF1DataNonIRIGFmt nonirig_fmt_;
    Ch10TDPComponent tdp_comp_;
    uint64_t loc_;
    const uint8_t* data_ptr_;
    const uint8_t* orig_data_ptr_;
    const uint8_t* body_ptr_;
    Ch10Status status_;
    Ch10Context ctx_;
    uint64_t abs_pos_;
    uint32_t pkt_size_;
    uint32_t body_size_;
    uint32_t rtc1_;
    uint32_t rtc2_;
    std::vector<uint8_t> data_;

    Ch10TDPComponentTest() : loc_(0), data_ptr_(nullptr), orig_data_ptr_(nullptr),
        status_(Ch10Status::NONE), body_ptr_(nullptr), ctx_(0), tdp_comp_(&ctx_), abs_pos_(0),
        pkt_size_(0), body_size_(0), rtc1_(0), rtc2_(0)
    {
        // Fill out values for fake tdp csdw data.
        tdp_fmt_.src = 0; // internal
        tdp_fmt_.time_fmt = 1; // IRIG-A
        tdp_fmt_.leap_year = 0;
        tdp_fmt_.date_fmt = 0; // IRIG
    }

    void CreateBuffer()
    {
        size_t csdw_size = tdp_comp_.tdp_csdw_elem.size;
        size_t data_size = csdw_size;
        if (tdp_fmt_.date_fmt == 0)
            data_size += tdp_comp_.tdp_irig_elem.size;
        else
            data_size += tdp_comp_.tdp_nonirig_elem.size;

        data_.resize(data_size);

        TDF1CSDWFmt* csdw_ptr = (TDF1CSDWFmt*)data_.data();
        *csdw_ptr = tdp_fmt_;

        if (tdp_fmt_.date_fmt == 0)
        {
            TDF1DataIRIGFmt* irig_ptr = (TDF1DataIRIGFmt*)(data_.data() + csdw_size);
            *irig_ptr = irig_fmt_;
        }
        else
        {
            TDF1DataNonIRIGFmt* nonirig_ptr = (TDF1DataNonIRIGFmt*)(data_.data() + csdw_size);
            *nonirig_ptr = nonirig_fmt_;
        }

        data_ptr_ = (const uint8_t*)data_.data();
    }
};

TEST_F(Ch10TDPComponentTest, ParseTDPNone)
{
    // Set the date format to irig time.
    tdp_fmt_.date_fmt = 0;

    // Set time_fmt to 0xf = none
    tdp_fmt_.time_fmt = 0xf;

    // Create a buffer with the csdw and irig date format.
    CreateBuffer();

    // Check that parsed values agree with intended values.
    status_ = tdp_comp_.Parse(data_ptr_, loc_);
    EXPECT_EQ((*tdp_comp_.tdp_csdw_elem.element)->time_fmt, tdp_fmt_.time_fmt);
    EXPECT_EQ((*tdp_comp_.tdp_csdw_elem.element)->date_fmt, tdp_fmt_.date_fmt);

    // Status must be TDP_NONE for time_fmt = none
    EXPECT_EQ(status_, Ch10Status::TDP_NONE);

    // Set src to none and time_fmt to 0, then confirm status again.
    tdp_fmt_.time_fmt = 0;
    tdp_fmt_.src = 0xf;
    CreateBuffer();
    status_ = tdp_comp_.Parse(data_ptr_, loc_);
    EXPECT_EQ(status_, Ch10Status::TDP_NONE);
}