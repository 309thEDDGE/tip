#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <cmath>
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
    uint64_t abs_time_;
    uint32_t pkt_size_;
    uint32_t body_size_;
    uint32_t rtc1_;
    uint32_t rtc2_;
    std::vector<uint8_t> data_;

    Ch10TDPComponentTest() : loc_(0), data_ptr_(nullptr), orig_data_ptr_(nullptr),
        status_(Ch10Status::NONE), body_ptr_(nullptr), ctx_(0), tdp_comp_(&ctx_), abs_pos_(0),
        pkt_size_(0), body_size_(0), rtc1_(0), rtc2_(0), abs_time_(0)
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

    void SetIRIGFormat()
    {
        // Milliseconds must be multiple of 10.
        uint16_t remainder, counts;

        uint16_t day = 198;
        MaxMultiplierCounts(day, 100, counts, remainder);
        irig_fmt_.HDn = counts;
        MaxMultiplierCounts(remainder, 10, counts, remainder);
        irig_fmt_.TDn = counts;
        MaxMultiplierCounts(remainder, 1, counts, remainder);
        irig_fmt_.Dn = counts;

        uint16_t hour = 22;
        MaxMultiplierCounts(hour, 10, counts, remainder);
        irig_fmt_.THn = counts;
        MaxMultiplierCounts(remainder, 1, counts, remainder);
        irig_fmt_.Hn = counts;

        uint16_t minute = 11;
        MaxMultiplierCounts(minute, 10, counts, remainder);
        irig_fmt_.TMn = counts;
        MaxMultiplierCounts(remainder, 1, counts, remainder);
        irig_fmt_.Mn = counts;

        uint16_t sec = 36;
        MaxMultiplierCounts(sec, 10, counts, remainder);
        irig_fmt_.TSn = counts;
        MaxMultiplierCounts(remainder, 1, counts, remainder);
        irig_fmt_.Sn = counts;

        uint16_t ms = 660;
        MaxMultiplierCounts(ms, 100, counts, remainder);
        irig_fmt_.Hmn = counts;
        MaxMultiplierCounts(remainder, 10, counts, remainder);
        irig_fmt_.Tmn = counts;

        abs_time_ = ComputeIRIGAbsTime(day, hour, minute, sec, ms);

    }

    void SetNonIRIGFormat()
    {
        // Milliseconds must be multiple of 10.
        uint16_t remainder, counts;

        uint16_t year = 2012;
        MaxMultiplierCounts(year, 1000, counts, remainder);
        nonirig_fmt_.OYn = counts;
        MaxMultiplierCounts(remainder, 100, counts, remainder);
        nonirig_fmt_.HYn = counts;
        MaxMultiplierCounts(remainder, 10, counts, remainder);
        nonirig_fmt_.TYn = counts;
        MaxMultiplierCounts(remainder, 1, counts, remainder);
        nonirig_fmt_.Yn = counts;

        uint16_t mth = 2;
        MaxMultiplierCounts(mth, 10, counts, remainder);
        nonirig_fmt_.TOn = counts;
        MaxMultiplierCounts(remainder, 1, counts, remainder);
        nonirig_fmt_.On = counts;

        uint16_t day = 19;
        MaxMultiplierCounts(day, 10, counts, remainder);
        nonirig_fmt_.TDn = counts;
        MaxMultiplierCounts(remainder, 1, counts, remainder);
        nonirig_fmt_.Dn = counts;

        uint16_t hour = 22;
        MaxMultiplierCounts(hour, 10, counts, remainder);
        nonirig_fmt_.THn = counts;
        MaxMultiplierCounts(remainder, 1, counts, remainder);
        nonirig_fmt_.Hn = counts;

        uint16_t minute = 11;
        MaxMultiplierCounts(minute, 10, counts, remainder);
        nonirig_fmt_.TMn = counts;
        MaxMultiplierCounts(remainder, 1, counts, remainder);
        nonirig_fmt_.Mn = counts;

        uint16_t sec = 36;
        MaxMultiplierCounts(sec, 10, counts, remainder);
        nonirig_fmt_.TSn = counts;
        MaxMultiplierCounts(remainder, 1, counts, remainder);
        nonirig_fmt_.Sn = counts;

        uint16_t ms = 660;
        MaxMultiplierCounts(ms, 100, counts, remainder);
        nonirig_fmt_.Hmn = counts;
        MaxMultiplierCounts(remainder, 10, counts, remainder);
        nonirig_fmt_.Tmn = counts;

        abs_time_ = ComputeNonIRIGAbsTime(year, mth, day, hour, minute, sec, ms);

    }

    void MaxMultiplierCounts(const uint16_t& value, uint16_t multiplier,
        uint16_t& counts, uint16_t& remainder)
    {
        counts = (value / multiplier);
        remainder = value % multiplier;
        /*printf("value %hu, mult %hu, counts %hu, remainder %hu\n", value, multiplier,
            counts, remainder);*/
    }

    uint64_t ComputeIRIGAbsTime(uint16_t day, uint16_t hour, uint16_t minute,
        uint16_t sec, uint16_t ms)
    {
        // Check day for 0 error.
        if (day == 0)
            day = 1;

        // Get seconds into the first year of the epoch.
        std::tm temptime = { 0 };
        temptime.tm_sec = sec;
        temptime.tm_min = minute;
        temptime.tm_hour = hour;
        temptime.tm_year = 70;
        temptime.tm_isdst = 0;
        temptime.tm_mday = day;
#ifdef __WIN64
        uint64_t rawtime = uint64_t(_mkgmtime64(&temptime));
#elif defined __linux__
        uint64_t rawtime = uint64_t(timegm(&temptime));
#else
#error OS Not Supported!
#endif

        // Convert to nanoseconds.
        rawtime = rawtime * uint64_t(1e9) + uint64_t(ms) * uint64_t(1e6);

        return rawtime;
    }

    uint64_t ComputeNonIRIGAbsTime(uint16_t year, uint16_t mth, uint16_t day,
        uint16_t hour, uint16_t minute, uint16_t sec, uint16_t ms)
    {
        // Get seconds since the epoch using mktime.
        std::tm temptime = { 0 };
        temptime.tm_sec = sec;
        temptime.tm_min = minute;
        temptime.tm_hour = hour;

        // Values from Ch10 may be [0, 30] and struct time takes
        // [1, 31] for tm_mday. This would explain why DRA shows 
        // day+1, where day is tm_mday recovered from data I parse.
        // Will add 1 to day below to correct this.
        // 
        // Note: the correction mentioned above is causing an off-by-one error 
        // in both doy and non-doy date and time computations. My only 
        // guess is that this correction and the above explanation is in
        // error. I now change day+1 to day for both.
        temptime.tm_mday = day;
        temptime.tm_mon = mth - 1;
        temptime.tm_year = year - 1900;
        temptime.tm_isdst = 0;
#ifdef __WIN64
        uint64_t rawtime = uint64_t(_mkgmtime64(&temptime));
#elif defined __linux__
        uint64_t rawtime = uint64_t(timegm(&temptime));
#else
#error OS Not Supported!
#endif

        // Convert to nanoseconds since the epoch and add milliseconds.
        rawtime = rawtime * uint64_t(1e9) + uint64_t(ms) * uint64_t(1e6);

        return rawtime;
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

    // The context must indicate that tdp is not valid.
    EXPECT_EQ(ctx_.tdp_valid, false);

    // Set src to none and time_fmt to 0, then confirm status again.
    tdp_fmt_.time_fmt = 0;
    tdp_fmt_.src = 0xf;
    CreateBuffer();
    status_ = tdp_comp_.Parse(data_ptr_, loc_);
    EXPECT_EQ(status_, Ch10Status::TDP_NONE);
    EXPECT_EQ(ctx_.tdp_valid, false);
}

TEST_F(Ch10TDPComponentTest, ComputeIRIGTime)
{
    // Set the date format to irig time.
    tdp_fmt_.date_fmt = 0;

    // Fill the IRIG time object.
    SetIRIGFormat();

    // Compare absolute time.
    uint64_t test_abs_time = tdp_comp_.ComputeIRIGTime((const TDF1DataIRIGFmt* const)&irig_fmt_);

    ASSERT_EQ(abs_time_, test_abs_time);
}

TEST_F(Ch10TDPComponentTest, ComputeNonIRIGTime)
{
    // Set the date format to non-irig time.
    tdp_fmt_.date_fmt = 1;

    // Fill the non-IRIG time object.
    SetNonIRIGFormat();

    // Compare absolute time.
    uint64_t test_abs_time = tdp_comp_.ComputeNonIRIGTime(
        (const TDF1DataNonIRIGFmt* const)&nonirig_fmt_);

    ASSERT_EQ(abs_time_, test_abs_time);
}

TEST_F(Ch10TDPComponentTest, ParseIRIGTime)
{
    // Set the date format to irig time.
    tdp_fmt_.date_fmt = 0;

    // Fill the IRIG time object.
    SetIRIGFormat();

    // Prepare the context.
    uint64_t abs_pos = 6112919;
    Ch10PacketHeaderFmt hdr_fmt;
    hdr_fmt.data_size = 3399;
    hdr_fmt.pkt_size = 4399;
    hdr_fmt.rtc1 = 321053;
    hdr_fmt.rtc2 = 502976;
    uint64_t rtc = ((uint64_t(hdr_fmt.rtc2) << 32) + uint64_t(hdr_fmt.rtc1)) * 100;
    uint8_t tdp_doy = 1; // 1 for irig time
    hdr_fmt.intrapkt_ts_source = 0;
    hdr_fmt.time_format = 1;

    ctx_.UpdateContext(abs_pos, &hdr_fmt);

    // Ch10Context::UpdateWithTDPData will be called by Parse().

    // Create a buffer with the csdw and irig date format.
    CreateBuffer();

    // Check that parsed values agree with intended values.
    status_ = tdp_comp_.Parse(data_ptr_, loc_);
    EXPECT_EQ((*tdp_comp_.tdp_csdw_elem.element)->time_fmt, tdp_fmt_.time_fmt);
    EXPECT_EQ((*tdp_comp_.tdp_csdw_elem.element)->date_fmt, tdp_fmt_.date_fmt);

    // Status must be ok if time_fmt and src are not none
    EXPECT_EQ(status_, Ch10Status::OK);

    // Confirm vars set by UpdateWithTDPData()
    EXPECT_EQ(ctx_.tdp_doy, tdp_doy);
    EXPECT_EQ(ctx_.tdp_abs_time, abs_time_);
    EXPECT_EQ(ctx_.tdp_rtc, rtc);
    EXPECT_EQ(ctx_.tdp_valid, true);
    EXPECT_EQ(ctx_.found_tdp, true);
}

TEST_F(Ch10TDPComponentTest, ParseNonIRIGTime)
{
    // Set the date format to non-irig time.
    tdp_fmt_.date_fmt = 1;

    // Fill the IRIG time object.
    SetNonIRIGFormat();

    // Prepare the context.
    uint64_t abs_pos = 6112919;
    Ch10PacketHeaderFmt hdr_fmt;
    hdr_fmt.data_size = 3399;
    hdr_fmt.pkt_size = 4399;
    hdr_fmt.rtc1 = 321053;
    hdr_fmt.rtc2 = 502976;
    uint64_t rtc = ((uint64_t(hdr_fmt.rtc2) << 32) + uint64_t(hdr_fmt.rtc1)) * 100;
    uint8_t tdp_doy = 0; // 0 for non-irig time
    hdr_fmt.intrapkt_ts_source = 0;
    hdr_fmt.time_format = 1;

    ctx_.UpdateContext(abs_pos, &hdr_fmt);

    // Ch10Context::UpdateWithTDPData will be called by Parse().

    // Create a buffer with the csdw and irig date format.
    CreateBuffer();

    // Check that parsed values agree with intended values.
    status_ = tdp_comp_.Parse(data_ptr_, loc_);
    EXPECT_EQ((*tdp_comp_.tdp_csdw_elem.element)->time_fmt, tdp_fmt_.time_fmt);
    EXPECT_EQ((*tdp_comp_.tdp_csdw_elem.element)->date_fmt, tdp_fmt_.date_fmt);

    // Status must be ok if time_fmt and src are not none
    EXPECT_EQ(status_, Ch10Status::OK);

    // Confirm vars set by UpdateWithTDPData()
    EXPECT_EQ(ctx_.tdp_doy, tdp_doy);
    EXPECT_EQ(ctx_.tdp_abs_time, abs_time_);
    EXPECT_EQ(ctx_.tdp_rtc, rtc);
    EXPECT_EQ(ctx_.tdp_valid, true);
    EXPECT_EQ(ctx_.found_tdp, true);
}