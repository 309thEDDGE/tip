#include "i106_parse_context.h"

I106ParseContext::I106ParseContext() : id_(UINT16_MAX), 
retcode_(0), rtc_to_ns_(100), ts_ptr_(nullptr),
msg_abstime_(0), ts_size_(sizeof(*ts_ptr_)),
ch10md_ptr_(nullptr), calculated_rtc_(0),
calculated_rtc_ref_(calculated_rtc_), output_path_("")
{
#ifdef DEBUG
	if (DEBUG > 2)
		printf("I106ParseContext(): TimeStamp size is %d\n", data_fmt_size_);
#endif
}

void I106ParseContext::Initialize(uint16_t ID, const Ch10MetaData* ch10md_ptr,
	const std::string& outpath)
{
	id_ = ID;
	ch10md_ptr_ = ch10md_ptr;
	output_path_ = outpath;
}

I106ParseContext::~I106ParseContext()
{}

bool I106ParseContext::InitializeWriter()
{
	return true;
}

void I106ParseContext::CalcRtc(const uint32_t& rtc1, const uint32_t& rtc2)
{
	calculated_rtc_ = ((uint64_t(rtc2) << 32) + uint64_t(rtc1)) * rtc_to_ns_;
}

void I106ParseContext::CalcRtcFromTs()
{
	calculated_rtc_ = ((ts_ptr_->ts2_ << 32) + ts_ptr_->ts1_) * rtc_to_ns_;
}

uint8_t I106ParseContext::CalcAbsTimeFromTs()
{
	return CalcAbsTimeDecision(true);
}

uint8_t I106ParseContext::CalcAbsTimeFromHdrRtc()
{
	return CalcAbsTimeDecision(false);
}

uint8_t I106ParseContext::CalcAbsTimeDecision(bool use_ts)
{
	// This function assumes that the pointer to TimeStamp points to
	// the correct location in the buffer where the time stamp bits are located.

	// time_fmt: 0 = IRIG106 Ch4 48-bit time format, 1 = IEEE-1588 format.
	// ts_source: 0 = 48-bit rtc, 1 = pkt secondary header time 

	if (ch10md_ptr_->intrapkt_ts_source_ == 1)
	{
		// Not implemented.
		printf("(%03u) I106ParseContext::parse_TS(): TS_source = 1 NOT IMPLEMENTED\n", id_);
		return 1;
	}
	else
	{
		switch (ch10md_ptr_->time_format_)
		{
		case 0:
			// 48-bit time format.
			if (use_ts)
			{
				CalcRtcFromTs();
				msg_abstime_ = ch10md_ptr_->timedatapkt_abstime_ +
					(calculated_rtc_ - ch10md_ptr_->timedatapkt_rtc_);
			}
			else
			{
				msg_abstime_ = ch10md_ptr_->timedatapkt_abstime_ +
					(ch10md_ptr_->header_rtc_ - ch10md_ptr_->timedatapkt_rtc_);
			}
			return 0;
		case 1:
			// IEEE-1588 time format.
			printf("(%03u) I106ParseContext::parse_TS(): TS_format = 1 NOT IMPLEMENTED\n", id_);
			return 1;
		case 2:
			printf("(%03u) I106ParseContext::parse_TS(): TS_format == 2 RESERVED\n", id_);
			return 1;
		case 3:
			printf("(%03u) I106ParseContext::parse_TS(): TS_format == 3 RESERVED\n", id_);
			return 1;
		default:
			return 1;
		}
	}
	return 0;
}
