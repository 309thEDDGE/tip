#ifndef PARQUET_VIDEODATAF0_H
#define PARQUET_VIDEODATAF0_H

#include "parquet_context.h"
#include "VideoDataF0Format.h"

const int DEFAULT_ROW_GROUP_COUNT_VIDEO = 10000;
const int DEFAULT_BUFFER_SIZE_MULTIPLIER_VIDEO = 10;

class ParquetVideoDataF0 : public ParquetContext
{
private:
	int max_temp_element_count_;
	int temp_element_count_;
	uint16_t id_;

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
	std::vector<uint8_t> PL_;

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
	*/
	std::vector<video_datum> video_data_;
	std::vector<int8_t>* saved_video_data_;
	
	/*
		label:
		Name of video if found in TMATS
	*/
	std::vector<std::string> label_;

	/*
		time:
		Measured in nano seconds from the epoch
		Depends on "doy" (see above comment)
	*/
	std::vector<uint64_t> time_;

	/*
		channel id:
		Channel id associated with the video payload
		Note: One channel ID per video stream
	*/
	std::vector<uint16_t> channel_id_;

public:
	ParquetVideoDataF0(std::string outfile, uint16_t ID, bool truncate);

	/*
		Writes the remaining rows that were not written by append_data
	*/
	void commit();

	/*
		Appends one video packet from the chapter 10
		Writes to parquet once the added entries sum to:
			DEFAULT_ROW_GROUP_COUNT* DEFAULT_BUFFER_SIZE_MULTIPLIER
	*/
	void append_data(const std::vector<uint64_t>& time_stamp,
		const uint8_t& doy,
		const std::string& label,
		const uint32_t& channel_id,
		const VideoDataF0ChanSpecFormat* vid_flags,
		const uint32_t& transport_stream_pkt_count,
		std::vector<video_datum>& data_vec);

};

#endif