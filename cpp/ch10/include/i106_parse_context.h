
/*
Note that for this template base class the full definition must
be located in the header file. (Actually, there are other methods
to avoid this. According to forum search, the other methods are
more complicated or less reliable.)
*/

#ifndef I106_PARSECONTEXT_H
#define I106_PARSECONTEXT_H

#include <cstdint>
#include <cstdio>
#include "binbuff.h"


class Ch10MetaData
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

	// Value of the Relative Time Counter (RTC),
	// a quantity extracted from each ch10 packet header.
	// Unit: nanosecond
	uint64_t header_rtc_;

	Ch10MetaData() : timedatapkt_rtc_(0), doy_(0), timedatapkt_abstime_(0),
		header_rtc_(0), intrapkt_ts_source_(UINT32_MAX), time_format_(2)
	{}
};

// Bit-field that describes the most common time format found in 
// Ch10 data packets. Note: No other time formats have not been observed.
class Ch10TimeStamp
{
public:
	uint64_t ts1_ : 32;
	uint64_t ts2_ : 32;
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

class I106ParseContext
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

	// Ch10MetaData
	const Ch10MetaData* ch10md_ptr_;

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
	const Ch10TimeStamp* ts_ptr_;


	// TimeStamp size, calculated in the constructor. Useful for 
	// calculations in preparation for buffer advancement. This
	// quantity is currently set to the size of the TimeStamp object.
	// Additional logic will be required when the derived class 
	// is called upon to parse a time stamp that is defined differently
	// than TimeStamp, as is possible per the Ch10 spec.
	const uint8_t ts_size_;

	// Output file path relevant to the derived class parse type.
	std::string output_path_;

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
	I106ParseContext();
	void Initialize(uint16_t ID, const Ch10MetaData* ch10md_ptr, 
		const std::string& outpath);

	// Virtual method to be defined in derived class. Mentioned 
	// here as a reminder to initialize the class that writes 
	// parsed data to disk or other medium.
	virtual bool InitializeWriter();

	// Create virtual destructor, even it does nothing. This ensures that 
	// derived class destructor functions as expected. 
	virtual ~I106ParseContext();

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

	// Return a pointer to the member ch10td_;
	//const Ch10TimeData* GetCh10TimeDataPtr() const;

	// Return a pointer to the member ch10hd_;
	//const Ch10HeaderData* GetCh10HeaderDataPtr() const;

};



#endif 
