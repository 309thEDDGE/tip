#include <vector>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ch10_status.h"
#include "ch10_time.h"

class Ch10TimeTest : public ::testing::Test
{
protected:
	Ch10Status status_;
	Ch10Time time_;
	uint64_t ns_time_;
	uint64_t expected_ns_time_;
	uint8_t time_format_;
	std::vector<uint8_t> time_data_;
	const uint8_t* data_ptr_;

	Ch10TimeTest() : time_(), ns_time_(0), expected_ns_time_(0), time_format_(0),
		time_data_({ 123, 31, 72, 192, 201, 17, 99, 56 }), data_ptr_(time_data_.data())
	{ }
};

TEST_F(Ch10TimeTest, CalculateRTCTimeFromComponents)
{
	uint32_t rtc1 = 39911;
	uint32_t rtc2 = 19601;
	expected_ns_time_ = ((uint64_t(rtc2) << 32) + uint64_t(rtc1)) * 
		time_.rtc_to_ns_;

	ns_time_ = time_.CalculateRTCTimeFromComponents(rtc1, rtc2);
	EXPECT_EQ(expected_ns_time_, ns_time_);
}

TEST_F(Ch10TimeTest, ParseSecondaryHeaderTimeFormats)
{
	time_format_ = 0;
	status_ = time_.ParseSecondaryHeaderTime(data_ptr_, time_format_, ns_time_);
	EXPECT_EQ(status_, Ch10Status::OK);

	time_format_ = 1;
	status_ = time_.ParseSecondaryHeaderTime(data_ptr_, time_format_, ns_time_);
	EXPECT_EQ(status_, Ch10Status::OK);

	time_format_ = 2;
	status_ = time_.ParseSecondaryHeaderTime(data_ptr_, time_format_, ns_time_);
	EXPECT_EQ(status_, Ch10Status::OK);

	time_format_ = 3;
	status_ = time_.ParseSecondaryHeaderTime(data_ptr_, time_format_, ns_time_);
	EXPECT_EQ(status_, Ch10Status::INVALID_SECONDARY_HDR_FMT);
}

TEST_F(Ch10TimeTest, ParseSecondaryHeaderTimeAdvancesPointer)
{
	const uint8_t* temp_ptr_ = data_ptr_;
	status_ = time_.ParseSecondaryHeaderTime(data_ptr_, time_format_, ns_time_);
	EXPECT_EQ(temp_ptr_ + 8, data_ptr_);
}

TEST_F(Ch10TimeTest, ParseSecondaryHeaderTimeCorrectFormat)
{
	const uint8_t* temp_ptr_ = data_ptr_;

	// binary weighted time
	time_format_ = 0;
	status_ = time_.ParseSecondaryHeaderTime(temp_ptr_, time_format_, ns_time_);
	EXPECT_EQ(status_, Ch10Status::OK);

	// subract 8 because temp_ptr_ is advanced by ParseSecondaryHeaderTime
	temp_ptr_ -= 8;
	time_.ParseBinaryWeightedTime(temp_ptr_, expected_ns_time_);
	EXPECT_EQ(expected_ns_time_, ns_time_);

	// IEEE 1588 time
	temp_ptr_ = data_ptr_;
	time_format_ = 1;
	status_ = time_.ParseSecondaryHeaderTime(temp_ptr_, time_format_, ns_time_);
	EXPECT_EQ(status_, Ch10Status::OK);
	temp_ptr_ -= 8;
	time_.ParseIEEE1588Time(temp_ptr_, expected_ns_time_);
	EXPECT_EQ(expected_ns_time_, ns_time_);

	// ERTC time
	temp_ptr_ = data_ptr_;
	time_format_ = 2;
	status_ = time_.ParseSecondaryHeaderTime(temp_ptr_, time_format_, ns_time_);
	EXPECT_EQ(status_, Ch10Status::OK);
	temp_ptr_ -= 8;
	time_.ParseERTCTime(temp_ptr_, expected_ns_time_);
	EXPECT_EQ(expected_ns_time_, ns_time_);
}

TEST_F(Ch10TimeTest, ParseBinaryWeightedTime)
{
	Ch10BinWtTimeStampFmt* binwt_data = (Ch10BinWtTimeStampFmt * )time_data_.data();
	binwt_data->microsec = 134;
	binwt_data->low_order = 53;
	binwt_data->high_order = 84112;
	expected_ns_time_ = uint64_t(binwt_data->microsec) + uint64_t(binwt_data->low_order * 0.01) * uint64_t(1e6)
		+ uint64_t(double(binwt_data->high_order) * 655.36) * uint64_t(1e6);
	// Now from microseconds to nanoseconds.
	expected_ns_time_ *= 1000;
	time_.ParseBinaryWeightedTime(data_ptr_, ns_time_);
	EXPECT_EQ(expected_ns_time_, ns_time_);

	// Now try with maximum values to ensure that full resolution
	// is handled.
	binwt_data->microsec = 9999; // max 9999
	binwt_data->low_order = 32767; // max 2^15 - 1
	binwt_data->high_order = 65535; // max 2^16 - 1
	expected_ns_time_ = uint64_t(binwt_data->microsec) + uint64_t(binwt_data->low_order * 0.01) * uint64_t(1e6)
		+ uint64_t(double(binwt_data->high_order) * 655.36) * uint64_t(1e6);
	// Now from microseconds to nanoseconds.
	expected_ns_time_ *= 1000;
	time_.ParseBinaryWeightedTime(data_ptr_, ns_time_);
	EXPECT_EQ(expected_ns_time_, ns_time_);
}

TEST_F(Ch10TimeTest, ParseIEEE1588Time)
{
	Ch10IEEE1588TimeStampFmt* ieee_data = (Ch10IEEE1588TimeStampFmt*)time_data_.data();
	ieee_data->ns_word = 498011309;
	ieee_data->sec_word = 91243066;
	expected_ns_time_ = uint64_t(ieee_data->ns_word) + uint64_t(ieee_data->sec_word) * 1e9;
	time_.ParseIEEE1588Time(data_ptr_, ns_time_);
	EXPECT_EQ(expected_ns_time_, ns_time_);
}

TEST_F(Ch10TimeTest, ParseERTCTime)
{
	Ch10ERTCTimeStampFmt* ertc_data = (Ch10ERTCTimeStampFmt*)time_data_.data();
	ertc_data->lslw = 498011309;
	ertc_data->mslw = 91243066;
	expected_ns_time_ = uint64_t(ertc_data->lslw) + (uint64_t(ertc_data->mslw) << 32);
	time_.ParseERTCTime(data_ptr_, ns_time_);
	EXPECT_EQ(expected_ns_time_, ns_time_);
}

TEST_F(Ch10TimeTest, ParseRTCTime)
{
	Ch10RTCTimeStampFmt* rtc_data = (Ch10RTCTimeStampFmt*)time_data_.data();
	rtc_data->ts1_ = 498011309;
	rtc_data->ts2_ = 91243066;
	expected_ns_time_ = ((uint64_t(rtc_data->ts2_) << 32) + uint64_t(rtc_data->ts1_)) * 100;
	time_.ParseRTCTime(data_ptr_, ns_time_);
	EXPECT_EQ(expected_ns_time_, ns_time_);
}

TEST_F(Ch10TimeTest, ParseIPTSRTCSource)
{
	const uint8_t* temp_ptr_ = data_ptr_;
	uint8_t intrapkt_src = 0;
	uint8_t time_fmt = 0;
	status_ = time_.ParseIPTS(temp_ptr_, ns_time_, intrapkt_src, time_fmt);
	EXPECT_EQ(status_, Ch10Status::OK);
	EXPECT_EQ(data_ptr_ + 8, temp_ptr_);

	temp_ptr_ -= 8;
	time_.ParseRTCTime(temp_ptr_, expected_ns_time_);
	EXPECT_EQ(expected_ns_time_, ns_time_);
}

TEST_F(Ch10TimeTest, ParseIPTSSecondaryHeaderSource)
{
	const uint8_t* temp_ptr_ = data_ptr_;
	uint8_t intrapkt_src = 1;
	uint8_t time_fmt = 1; // IEEE 1588 format
	status_ = time_.ParseIPTS(temp_ptr_, ns_time_, intrapkt_src, time_fmt);
	EXPECT_EQ(status_, Ch10Status::OK);
	EXPECT_EQ(data_ptr_ + 8, temp_ptr_);

	temp_ptr_ -= 8;
	time_.ParseIEEE1588Time(temp_ptr_, expected_ns_time_);
	EXPECT_EQ(expected_ns_time_, ns_time_);
}

TEST_F(Ch10TimeTest, ParseIPTSInvalidSource)
{
	uint8_t intrapkt_src = 3; // not allowed > 2
	uint8_t time_fmt = 1; // IEEE 1588 format
	status_ = time_.ParseIPTS(data_ptr_, ns_time_, intrapkt_src, time_fmt);
	EXPECT_EQ(status_, Ch10Status::INVALID_INTRAPKT_TS_SRC);
}