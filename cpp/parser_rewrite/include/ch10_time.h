
#ifndef CH10_TIME_H_
#define CH10_TIME_H_

#include<cstdint>
#include "ch10_status.h"
#include "spdlog/spdlog.h"

class Ch10RTCTimeStampFmt
{
public:
    uint64_t ts1_ : 32;
    uint64_t ts2_ : 32;
};

class Ch10BinWtTimeStampFmt
{
public:
    uint16_t : 16;
    uint16_t microsec;
    uint16_t low_order;
    uint16_t high_order;

};

class Ch10IEEE1588TimeStampFmt
{
public:
    uint32_t ns_word;
    uint32_t sec_word;
};

class Ch10ERTCTimeStampFmt
{
public:
    uint32_t lslw;
    uint32_t mslw;
};

/*
Handle all parsing and time conversion of data structures
within the Chapter 10.
*/

class Ch10Time
{
private:
    const Ch10RTCTimeStampFmt* rtc_time_ptr_;
    const Ch10BinWtTimeStampFmt* binwt_time_ptr_;
    const Ch10IEEE1588TimeStampFmt* ieee_time_ptr_;
    const Ch10ERTCTimeStampFmt* ertc_time_ptr_;

    uint64_t temp_ns_time_;

    // Binary weighted time calculation params
    uint64_t high_order_ns_;
    uint64_t low_order_ns_;

public:

    // Conversion factor for relative time counter (RTC)
    // to nanoseconds.
    // Unit: count/ns
    const uint64_t rtc_to_ns_;

    // Standard size in bytes of formatted time data in the Ch10
    const size_t time_data_size_;

    Ch10Time();

    /*
    Calculate the RTC time in nanoseconds since last counter reset from
    two 32-bit components.

    Args:
        rtc1    --> The first (most significant) 32-bit RTC word
        rtc2    --> The second (least significant) 32-bit RTC word

    Return:
        RTC time in nanoseconds
    */
    uint64_t& CalculateRTCTimeFromComponents(const uint32_t& rtc1, const uint32_t& rtc2);

    /*
    Parse raw binary data according the Ch10 packet secondary header criteria, time
    component only. The data pointer is advanced by 8 bytes, the standard size of the 
    Ch10 packet secondary header.

    Args:
        data        --> pointer to a position in the binary data which is currently
					    expected to contain a time stamp formatted according to 
                        Ch10 packet secondary header
        time_format --> Time format enum found in Ch10PacketHeaderFmt::time_format
        time_ns     --> Output time stamp in nanosecond units

    Return:
        Ch10Status
    */
    Ch10Status ParseSecondaryHeaderTime(const uint8_t*& data, uint8_t time_format,
        uint64_t& time_ns);

    /*
    Calculate the binary weighted time in nanosecond units with the assumption
    that the data pointer is positioned at a block of memory in which 
    binary weighted time is recorded.

    It is not known if this is absolute time or relative, though it's 
    likely absolute since IEEE 1588 time is absolute (epoch). 

    Args:
        data    --> pointer to a position in the binary data which points
                    to binary weighted time
        time_ns --> Output time stamp in nanosecond units
    */
    void ParseBinaryWeightedTime(const uint8_t*& data, uint64_t& time_ns);

    /*
    Calculate the IEEE 1588 time in nanosecond units with the assumption
    that the data pointer is positioned at a block of memory in which
    IEEE 1588 time is recorded. Parsed time is believed to be absolute though
    this parser has not been validated against real data.

    Args:
        data    --> pointer to a position in the binary data which points
                    appropriately formatted time block
        time_ns --> Output time stamp in nanosecond units
    */
    void ParseIEEE1588Time(const uint8_t*& data, uint64_t& time_ns);

    /*
    Calculate the ERTC time in nanosecond units with the assumption
    that the data pointer is positioned at a block of memory in which
    ERTC time is recorded. Parsed time is believed to be relative though
    this parser has not been validated against real data.

    Args:
        data    --> pointer to a position in the binary data which points
                    appropriately formatted time block
        time_ns --> Output time stamp in nanosecond units
    */
    void ParseERTCTime(const uint8_t*& data, uint64_t& time_ns);

    /*
    Calculate the RTC time in nanosecond units with the assumption
    that the data pointer is positioned at a block of memory in which
    RTC time is recorded. Parsed time is relative.

    Args:
        data    --> pointer to a position in the binary data which points
                    appropriately formatted time block
        time_ns --> Output time stamp in nanosecond units
    */
    void ParseRTCTime(const uint8_t*& data, uint64_t& time_ns);

    Ch10Status ParseIPTS(const uint8_t*& data, uint64_t& time_ns,
        uint8_t intrapkt_ts_src, uint8_t time_fmt);
};

#endif