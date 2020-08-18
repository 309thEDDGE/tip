#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parse_context.h"

class Fmt
{
	uint32_t val1_;
	uint32_t val2_;
};

enum class Status : uint8_t
{
	on_ = 1,
	off_ = 0,
};

class ParseContextDerived : public ParseContext<Fmt, Status>
{

public:
	TimeStamp derived_ts_;
	ParseContextDerived(BinBuff& buff, uint16_t ID): ParseContext(buff, ID), derived_ts_()
	{ }
	
	uint8_t Parse();
	//uint64_t RTC_NS() { return rtc_ns_;  }
	//void DerivedCalcRtcFromTs() { CalcRtcFromTs(); }
	void DerivedCalcAbsTimeFromTs() { CalcAbsTimeFromTs(); }
	void DerivedCalcAbsTimeFromHdrRtc() { CalcAbsTimeFromHdrRtc(); }
	uint64_t AbsTime() { return msg_abstime_; }
};

uint8_t ParseContextDerived::Parse()
{
	// Set internal TS.
	ts_ptr_ = &derived_ts_;

	// Set the Ch10TimeData and Ch10HeaderData pointers to the
	// internal objects.
	ch10td_ptr_ = &ch10td_;
	ch10hd_ptr_ = &ch10hd_;

	// Use common, i.e., only observed source and format.
	ch10hd_.intrapkt_ts_source_ = 0;
	ch10hd_.time_format_ = 0;

	ch10td_.timedatapkt_abstime_ = 34969829;
	ch10td_.timedatapkt_rtc_ = 11001189;
	ch10hd_.header_rtc_ = 25099;
	return uint8_t(0);
}

class ParseContextTest : public ::testing::Test
{
protected:

	BinBuff bb_;
	uint16_t id_;
	TimeStamp TS_;
	ParseContextDerived pcd_;
	ParseContextTest() : bb_(), id_(0), TS_(), pcd_(bb_, id_)
	{
	}
	/*void SetUp() override
	{

	}*/

	void TearDown() override
	{
		//remove(temp_file_name_.c_str());
	}

};

TEST_F(ParseContextTest, CalcRtc)
{
	// Set the test fixture TimeStamp object.
	TS_.ts1_ = 50361;
	TS_.ts2_ = 1006732;
	uint64_t expected = 432388101588703300;

	// Feed the values to CalcRtc()
	pcd_.CalcRtc(TS_.ts1_, TS_.ts2_);
	ASSERT_EQ(pcd_.calculated_rtc_ref_, expected);
}

TEST_F(ParseContextTest, CalcRtcFromTs)
{
	pcd_.Parse();

	// Set the TimeStamp object to which the internal
	// ts_ptr_ member points.
	pcd_.derived_ts_.ts1_ = 50361;
	pcd_.derived_ts_.ts2_ = 1006732;
	uint64_t expected = 432388101588703300;

	// Calculate RTC from internal TimeStamp object.
	pcd_.CalcRtcFromTs();
	ASSERT_EQ(pcd_.calculated_rtc_ref_, expected);
}

TEST_F(ParseContextTest, ParseTs)
{
	pcd_.Parse();

	// Set the TimeStamp object to which the internal
	// ts_ptr_ member points.
	pcd_.derived_ts_.ts1_ = 50361;
	pcd_.derived_ts_.ts2_ = 1006732;

	// Execute the function that calculates the absolute time
	// from internal members' data (ch10td_, instance of Ch10TimeData
	// and ch10hd_, instance of Ch10HeaderData) and the data to which
	// the ts_ptr_ points.
	pcd_.DerivedCalcAbsTimeFromTs();
	uint64_t expected = 432388101612671940;
	ASSERT_EQ(pcd_.AbsTime(), expected);
}

TEST_F(ParseContextTest, ParseTsFromPacketRtc)
{
	pcd_.Parse();

	// Execute the function that calculates the absolute time
	// from internal members' data (ch10td_, instance of Ch10TimeData
	// and ch10hd_, instance of Ch10HeaderData). The current RTC
	// value is pulled from header data (ch10hd_ptr_->header_rtc_)
	// set during initialization or, in this case, the custom Parse() method. 
	pcd_.DerivedCalcAbsTimeFromHdrRtc();
	uint64_t expected = 23993739;
	ASSERT_EQ(pcd_.AbsTime(), expected);
}
