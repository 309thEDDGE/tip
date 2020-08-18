
/*
Note that for this template base class the full definition must 
be located in the header file. (Actually, there are other methods
to avoid this. According to forum search, the other methods are
more complicated or less reliable.)
*/

#ifndef PARSECONTEXT_H
#define PARSECONTEXT_H

#include <cstdint>
#include <cstdio>
#include "binbuff.h"

class Ch10TimeData
{
public:
	// Value of the relative time counter (RTC)
	// as found in the Time Data Packet (TDP).
	// Unit: nanosecond
	uint64_t timedatapkt_rtc_;

	// Indicates if the absolute time is day of year (doy), value 1,
	// or value 0 otherwise.
	// Unit: bool as integer
	uint8_t doy_;

	// Relative to the beginning
	// of the year in an arbitrary year if doy = 1, else relative
	// to the epoch, the zeroth ns on Jan 1, 1970. Absolute
	// time as extracted from the TDP.
	// Unit: nanosecond
	uint64_t timedatapkt_abstime_;
};

class Ch10HeaderData
{
public:
	// Value of the Relative Time Counter (RTC),
	// a quantity extracted from each ch10 packet header.
	// Unit: nanosecond
	uint64_t header_rtc_;

	// A Ch10 defined value:
	// 0 = Packet Header 48-Bit Relative Time Counter.
	// 1 = Packet Secondary Header Time(Bit 7 must be 1).
	// Unit: number
	uint32_t intrapkt_ts_source_;

	// A Ch10 defined value:
	// 00 = IRIG 106 Chapter 4 binary weighted 48-bit time format. The
	//		two LSB’s of the 64 - bit Packet Secondary Header Time and
	//		Intra - Packet Time Stamp shall be zero filled.
	// 01 = IEEE - 1588 Time format.The Packet Secondary Header Time
	//		and each Intra - Packet Time Stamp shall contain a 64 - bit
	//		timestamp represented in accordance with the Time
	//		Representation type as specified by IEEE STD 1588 - 2002. The
	//		32 bits indicating seconds shall be placed into the Most
	//		Significant Long Word portion of the secondary header and the
	//		32 bits indicating nanoseconds shall be placed into the Least
	//		Significant Long Word portion.
	// 10 = Reserved
	// 11 = Reserved
	// Unit: number
	uint32_t time_format_;

	// Contains a value representing the Packet Channel ID.
	// All channels in a system must have a unique Channel ID for each data source.
	// Unit: number
	uint32_t channel_id_;

	// Size of packet body = total packet size - header size - footer size
	// Unit: byte
	uint32_t pkt_body_size_;
};

// Bit-field that describes the most common time format found in 
// Ch10 data packets. Note: No other time formats have not been observed.
class TimeStamp
{
public:
	uint64_t ts1_ : 32;
	uint64_t ts2_ : 32;
};

// State of the ParseContext base class.
enum class ParseStatus : uint8_t
{
	TS_NOT_IMPL = 0,
	TS_RESERVED = 1,
	TS_NOT_HANDLED = 2,
};

// ParseContext is the base class for implementing
// Ch10 packet parsers. The pure virtual function 
// Parse() is intended to parse the body of a ch10 
// packet for which the derived class it's called in
// is designed. The binary buffer position is assumed to be 
// set to the beginning of the
// packet body when Parse() is called. BinBuff function 
// calls are made within 
// the Parse() method to move the buffer
// read position as necessary to parse all relative
// parts of the ch10 packet. 
//
// Tfmt ought to be a bit-field or regular class that defines
// the bit-level format of the data payload relevant to the 
// derived class purpose or packet type. Derived classes
// may need to use multiple bit-field objects to deconstruct
// various headers, etc. Attempt to use only the object
// relevant to the primary data payload in the Tfmt template
// argument, in which case the ::Data() method has the implicated
// meaning.
//
// Tstat ought to be a custom enum class that defines the various
// status levels for the given packet defined in Parse()
// in the derived class. The ::Status() method gives the
// current value of the status for the given derived class.
template<class Tfmt, typename Tstat>
class ParseContext
{
	// Todo:
	// Create generic debug print info such as Ch10VideoDataF0::print_video_pkt_info()
	// for all derived classes to use.
	//
private:
	// Conversion factor for relative time counter (RTC)
	// to nanoseconds.
	// Unit: count/ns
	const uint64_t rtc_to_ns_;

	// Helper function that selects the correct time calculation
	// algorithm from Ch10HeaderData.intrapkt_ts_source_, 
	// Ch10HeaderData.time_format_ and the boolean argument
	// that indicates the pointer to TimeStamp, TS_, ought
	// to be used (use_ts = true) or the Ch10HeaderDAta.header_rtc_
	// relative time counter.
	uint8_t CalcAbsTimeDecision(bool use_ts);

	// RTC calculated by either CalcRtc()
	// or  CalcRtcFromTs().
	// Unit: nanosecond
	uint64_t calculated_rtc_;

protected:

	// Bit-field or class describing the format of the data payload
	// for which the derived class is designed to parse.
	const Tfmt* data_fmt_ptr_;

	// Enum class giving the custom status of the derived class.
	Tstat status_;

	// Pointer to instance of BinBuff object. Buffer
	// of the binary data on which the derived class
	// operates.
	BinBuff* bb_ptr_;

	// Ch10TimeData
	Ch10TimeData ch10td_;
	const Ch10TimeData* ch10td_ptr_;

	// Ch10HeaderData
	Ch10HeaderData ch10hd_;
	const Ch10HeaderData* ch10hd_ptr_;
	
	// Absolute time calculated with values pointed
	// to by ch10td_ptr_ and ch10hd_ptr_. Can be either
	// the entire packet absolute time or a sub-packet
	// absolute time, determined by the function called
	// to calculate this value.
	// See CalcAbsTimeFromTs() and CalcAbsTimeFromHdrRtc().
	uint64_t msg_abstime_;

	// Arbitrary ID assigned by the code that instantiates the 
	// class that derives from this base class. Used to track
	// threads.
	uint16_t id_;

	// Class member to be used as an internal code, if needed.
	uint8_t retcode_;

	// Size of the Tfmt object, computed in the constructor. 
	// This quantity is useful in calculations in preparation
	// for advancement of the buffer position to move to different parts
	// of a packet. 
	// Unit: count of bytes
	size_t data_fmt_size_;

	// Does not point to any internal TimeStamp object. Can be set
	// within the derived Parse() method interpret a specific position
	// in the buffer as a time stamp. Other time stamp types will
	// eventually need to be defined as Ch10s are found and parsed
	// that utilize other time formats--an occurrence that hasn't 
	// happened yet.
	const TimeStamp* ts_ptr_;

	// The ParseContext status indicator. 
	ParseStatus parse_status_;

	// TimeStamp size, calculated in the constructor. Useful for 
	// calculations in preparation for buffer advancement. This
	// quantity is currently set to the size of the TimeStamp object.
	// Additional logic will be required when the derived class 
	// is called upon to parse a time stamp that is defined differently
	// than TimeStamp, as is possible per the Ch10 spec.
	const uint8_t ts_size_;

	// Uses the TS_source and TS_format values to determine the
	// correct means of parsing the time stamp. This function does
	// not require the user to understand the various time formats.
	// The result, absolute time (doy=0) or day-of-year time (doy=1)
	// in nanoseconds is stored in absolute_msg_TS. 
	uint8_t CalcAbsTimeFromTs();

	// Similar calculation as above except uses the value of
	// pkt_hdr_RTC in place of calculating rtc_ns_ from the
	// data pointed to by TS_. 
	uint8_t CalcAbsTimeFromHdrRtc();
	
public:
	const uint64_t& calculated_rtc_ref_;

	// Pass the binary buffer containing the data that are to be
	// parsed. ID is used to identify the calling thread. 
	ParseContext(BinBuff& buff, uint16_t ID);
	
	// Create virtual destructor, even it does nothing. This ensures that 
	// derived class destructor functions as expected. 
	virtual ~ParseContext();
	
	// Must be overridden in derived class. The design assumes that
	// the binary buffer position has been set such that the parse
	// method can correctly interpret the bits.
	virtual uint8_t Parse() = 0;

	// Assumes that all or most Ch10 packets will need information about time,
	// given in Ch10TimeData, or Ch10 packet header data, given in Ch10HeaderData.
	// Pass null pointer (nullptr) if data aren't available such as when parsing
	// a packet header, which does need any time metadata to proceed and does
	// not rely on previous packet header data.
	virtual void Initialize(const Ch10TimeData* ch10td, const Ch10HeaderData* ch10hd); 
	
	// Return a pointer to the class or bit-field that is set to specific
	// locations in the buffer to effectively parse the data. It is the 
	// responsibility of the Parse method in the derived class to correctly
	// set the pointer.
	//
	// The validity of the data pointed to by the return value is dependent
	// upon being set correctly at the time of the call to this method. 
	const Tfmt* Data();

	// Return the current derived class status enum. The enum type is set
	// by the second template argument supplied in the definition of the 
	// derived class to this base template class.
	const Tstat& Status();

	// Return the current BinBuff position.
	const uint64_t& Position();

	// Calculate the Relative Time Counter in nanoseconds from
	// the two components of the RTC parsed from the raw data.
	// Not all Ch10 data will utilize an RTC of this format. 
	// Saves the result in rtc_ns_. 
	void CalcRtc(const uint32_t& rtc1, const uint32_t& rtc2);

	// Same result as the function CalcRtc() (see below). This 
	// method assumes that the pointer to TimeStamp (TS_) points
	// to the location within the buffer where a RTC time stamp
	// is located and uses the members of TS_ to complete the 
	// calculation. The result is stored in rtc_ns_. Also uses
	// the current values of absolute_pkt_TS and absolute_TS_rtc
	// to complete the calculation.
	void CalcRtcFromTs();

	// Set the BinBuff pointer to a new value.
	void SetBinbuff(BinBuff*);

	// Return a pointer to the member ch10td_;
	const Ch10TimeData* GetCh10TimeDataPtr() const;

	// Return a pointer to the member ch10hd_;
	const Ch10HeaderData* GetCh10HeaderDataPtr() const;
};

template<class Tfmt, typename Tstat>
ParseContext<Tfmt, Tstat>::ParseContext(BinBuff& buff, uint16_t ID) : bb_ptr_(&buff),
	data_fmt_size_(sizeof(*data_fmt_ptr_)), data_fmt_ptr_(NULL), id_(ID), retcode_(0), rtc_to_ns_(100),
	status_(), ts_ptr_(nullptr), 
	msg_abstime_(0), ts_size_(sizeof(*ts_ptr_)),
	ch10td_(), ch10hd_(), ch10hd_ptr_(nullptr), ch10td_ptr_(nullptr), calculated_rtc_(0),
	calculated_rtc_ref_(calculated_rtc_)
{
	#ifdef DEBUG
	if (DEBUG > 2)
		printf("ParseContext(): TimeStamp size is %d\n", data_fmt_size_);
	#endif
}

template<class Tfmt, typename Tstat>
void ParseContext<Tfmt, Tstat>::Initialize(const Ch10TimeData* ch10td, const Ch10HeaderData* ch10hd)
{
	ch10td_ptr_ = ch10td;
	ch10hd_ptr_ = ch10hd;
}

template<class Tfmt, typename Tstat>
ParseContext<Tfmt, Tstat>::~ParseContext<Tfmt, Tstat>()
{}

template<class Tfmt, typename Tstat>
const Tfmt* ParseContext<Tfmt, Tstat>::Data()
{ return data_fmt_ptr_; }

template<class Tfmt, typename Tstat>
const Tstat& ParseContext<Tfmt, Tstat>::Status()
{ return status_; }

template<class Tfmt, typename Tstat>
const uint64_t& ParseContext<Tfmt, Tstat>::Position()
{ return bb_ptr_->position_; }

template<class Tfmt, typename Tstat>
void ParseContext<Tfmt, Tstat>::CalcRtc(const uint32_t& rtc1, const uint32_t& rtc2)
{
	calculated_rtc_ = ((uint64_t(rtc2) << 32) + uint64_t(rtc1)) * rtc_to_ns_;
}

template<class Tfmt, typename Tstat>
void ParseContext<Tfmt, Tstat>::CalcRtcFromTs()
{
	calculated_rtc_ = ((ts_ptr_->ts2_ << 32) + ts_ptr_->ts1_) * rtc_to_ns_;
}

template<class Tfmt, typename Tstat>
void ParseContext<Tfmt, Tstat>::SetBinbuff(BinBuff* bb)
{
	bb_ptr_ = bb;
}

template<class Tfmt, typename Tstat>
uint8_t ParseContext<Tfmt, Tstat>::CalcAbsTimeFromTs()
{
	return CalcAbsTimeDecision(true);
}

template<class Tfmt, typename Tstat>
uint8_t ParseContext<Tfmt, Tstat>::CalcAbsTimeFromHdrRtc()
{
	return CalcAbsTimeDecision(false);
}

template<class Tfmt, typename Tstat>
uint8_t ParseContext<Tfmt, Tstat>::CalcAbsTimeDecision(bool use_ts)
{
	// This function assumes that the pointer to TimeStamp points to
	// the correct location in the buffer where the time stamp bits are located.

	// time_fmt: 0 = IRIG106 Ch4 48-bit time format, 1 = IEEE-1588 format.
	// ts_source: 0 = 48-bit rtc, 1 = pkt secondary header time 

	if (ch10hd_ptr_->intrapkt_ts_source_ == 1)
	{
		// Not implemented.
		printf("(%03u) ParseContext::parse_TS(): TS_source = 1 NOT IMPLEMENTED\n", id_);
		parse_status_ = ParseStatus::TS_NOT_IMPL;
		return 1;
	}
	else
	{
		switch (ch10hd_ptr_->time_format_)
		{
		case 0:
			// 48-bit time format.
			if (use_ts)
			{
				CalcRtcFromTs();
				msg_abstime_ = ch10td_ptr_->timedatapkt_abstime_ +
					(calculated_rtc_ - ch10td_ptr_->timedatapkt_rtc_);
			}
			else
			{
				msg_abstime_ = ch10td_ptr_->timedatapkt_abstime_ +
					(ch10hd_ptr_->header_rtc_ - ch10td_ptr_->timedatapkt_rtc_);
			}
			return 0;
		case 1:
			// IEEE-1588 time format.
			printf("(%03u) ParseContext::parse_TS(): TS_format = 1 NOT IMPLEMENTED\n", id_);
			parse_status_ = ParseStatus::TS_NOT_IMPL;
			return 1;
		case 2:
			printf("(%03u) ParseContext::parse_TS(): TS_format == 2 RESERVED\n", id_);
			parse_status_ = ParseStatus::TS_RESERVED;
			return 1;
		case 3:
			printf("(%03u) ParseContext::parse_TS(): TS_format == 3 RESERVED\n", id_);
			parse_status_ = ParseStatus::TS_RESERVED;
			return 1;
		default:
			parse_status_ = ParseStatus::TS_NOT_HANDLED;
			return 1;
		}
	}
	return 0;
}

template<class Tfmt, typename Tstat>
const Ch10TimeData* ParseContext<Tfmt, Tstat>::GetCh10TimeDataPtr() const
{
	return (const Ch10TimeData*)&ch10td_;
}

template<class Tfmt, typename Tstat>
const Ch10HeaderData* ParseContext<Tfmt, Tstat>::GetCh10HeaderDataPtr() const
{
	return (const Ch10HeaderData*)&ch10hd_;
}

#endif 
