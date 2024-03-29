#ifndef PARQUET_VIDEODATAF0_H
#define PARQUET_VIDEODATAF0_H

#include <cmath>
#include <vector>
#include <string>
#include "sysexits.h"
#include "parquet_context.h"
#include "managed_path.h"
#include "ch10_videof0_header_format.h"
#include "spdlog/spdlog.h"

class ParquetVideoDataF0
{
   private:
    int max_temp_element_count_;
	ParquetContext* pq_ctx_;

   public:
    std::string outfile_;
	static const int DEFAULT_ROW_GROUP_COUNT_VIDEO;
	static const int DEFAULT_BUFFER_SIZE_MULTIPLIER_VIDEO;

    uint16_t thread_id_;

    // Note: refer to the IRIG106 ch10 spec for additional information on many of these
    //		 parameters
    /*
		day of year: 
		true	-> time stamp given in nano seconds from 1970 (year not provided in ch10)					
		false	-> time stamp given in nano seconds from EPOCH (year provided in ch10) 
	*/
    std::vector<uint8_t> doy_;

    /*
		Embedded Time:
		Indicates if embedded time is present in the MPEG-2 video data
	*/
    std::vector<uint8_t> ET_;

    /*
		Intra-Packet Header:
		Indicates if intra-packet time stamps are inserted before each transport packet
	*/
    std::vector<uint8_t> IPH_;

    /*
		KLV metadata present in MPEG-2 video:
	*/
    std::vector<uint8_t> KLV_;

    /*
		payload:
		0		-> MPEG-2 MP@ML
		1		-> H.264 MP@L2.1
		2		-> H.264 MP@L2.2
		3		-> H.264 MP@L3
		4-15	-> reserved
	*/
    std::vector<int16_t> PL_;

    /*
		SCR/RTC Sync:
		false -> SCR is not synchronized with the 10 MHz RTC
		true -> SCR is synchronized with the 10 MHz RTC 
	*/
    std::vector<uint8_t> SRS_;

    /*
		video data:
		Video transport stream data
		There are 188 bytes of transport stream data per row 
		written out to the parquet file

		Use int32_t to avoid cast from uint16_t.
	*/
    std::vector<int32_t> video_data_;

    /*
		time:
		Measured in nano seconds from the epoch
		Depends on "doy" (see above comment)
	*/
    std::vector<int64_t> time_;

    /*
		channel id:
		Channel id associated with the video payload
		Note: One channel ID per video stream
	*/
    std::vector<int32_t> channel_id_;

      // Static functions which return static const data 
      // remove the need to declare exports when building
      // dynamic libraries in Windows. 
      static int GetRowGroupRowCount();
      static int GetRowGroupBufferCount();


    ParquetVideoDataF0(ParquetContext* parquet_context);
    int Initialize(ManagedPath outfile, uint16_t thread_id);

    /*
		Appends one video packet from the chapter 10
		Writes to parquet once the added entries sum to:
			DEFAULT_ROW_GROUP_COUNT* DEFAULT_BUFFER_SIZE_MULTIPLIER
	*/
    void Append(
        const uint64_t& time_stamp,
        const uint8_t& doy,
        const uint32_t& channel_id,
        const Ch10VideoF0HeaderFormat& vid_flags,
        const video_datum* const data);
};

#endif
