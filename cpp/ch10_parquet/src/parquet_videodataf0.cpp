#include "parquet_videodataf0.h"

ParquetVideoDataF0::ParquetVideoDataF0(ManagedPath outfile, uint16_t ID, bool truncate) :
	max_temp_element_count_(DEFAULT_ROW_GROUP_COUNT_VIDEO * DEFAULT_BUFFER_SIZE_MULTIPLIER_VIDEO),
	ParquetContext(DEFAULT_ROW_GROUP_COUNT_VIDEO), id_(ID), temp_element_count_(0)
{
	// Allocate vector memory. 	
	doy_.resize(max_temp_element_count_);
	ET_.resize(max_temp_element_count_);
	IPH_.resize(max_temp_element_count_);
	KLV_.resize(max_temp_element_count_);
	PL_.resize(max_temp_element_count_);
	SRS_.resize(max_temp_element_count_);

	/*
		Each video packet contains is 188 bytes of transport stream data
		Storing the video payload as uint16, the vector needs to be of size 188/2 = 94
	*/
	video_data_.resize(max_temp_element_count_ * TransportStream_DATA_COUNT); 
	time_.resize(max_temp_element_count_);
	channel_id_.resize(max_temp_element_count_);
	

	// Add fields to table.
	AddField(arrow::boolean(), "doy");
	AddField(arrow::boolean(), "ET");
	AddField(arrow::boolean(), "IPH");
	AddField(arrow::boolean(), "KLV");
	AddField(arrow::int16(), "PL");
	AddField(arrow::boolean(), "SRS");
	AddField(arrow::int32(), "data", TransportStream_DATA_COUNT);
	AddField(arrow::int64(), "time");
	AddField(arrow::int32(), "channelid");
	
	// Set memory locations.
	SetMemoryLocation<uint8_t>(doy_, "doy");
	SetMemoryLocation<uint8_t>(ET_, "ET");
	SetMemoryLocation<uint8_t>(IPH_, "IPH");
	SetMemoryLocation<uint8_t>(KLV_, "KLV");
	SetMemoryLocation<uint8_t>(PL_, "PL");
	SetMemoryLocation<uint8_t>(SRS_, "SRS");
	SetMemoryLocation<video_datum>(video_data_, "data");
	SetMemoryLocation<uint64_t>(time_, "time");
	SetMemoryLocation<uint16_t>(channel_id_, "channelid");

	bool ret = OpenForWrite(outfile.string(), truncate);
}

void ParquetVideoDataF0::commit()
{
#ifdef DEBUG
#if DEBUG > 1
	printf("(%03u) ParquetMilStd1553F1::commit()\n", id_);
#endif
#endif

	SPDLOG_INFO("({:02d}) commit(): Writing VideoDataF0 to Parquet, {:d} rows", id_, temp_element_count_);

	if (temp_element_count_ > 0)
	{
		int n_calls = int(std::ceil(double(temp_element_count_) / double(DEFAULT_ROW_GROUP_COUNT_VIDEO)));
		for (int i = 0; i < n_calls; i++)
		{
			if (i == n_calls - 1)
			{
				WriteColumns(temp_element_count_ - (n_calls - 1) * DEFAULT_ROW_GROUP_COUNT_VIDEO, 
					i * DEFAULT_ROW_GROUP_COUNT_VIDEO);
			}
			else
			{
				WriteColumns(DEFAULT_ROW_GROUP_COUNT_VIDEO, i * DEFAULT_ROW_GROUP_COUNT_VIDEO);
			}
		}

		std::fill(video_data_.begin(), video_data_.end(), 0);
	}
}

void ParquetVideoDataF0::append_data(const std::vector<uint64_t>& time_stamp, 
	const uint8_t& doy,
	const uint32_t& channel_id, 
	const VideoDataF0ChanSpecFormat* vid_flags, 
	const uint32_t& transport_stream_pkt_count, 
	std::vector<video_datum>& data_vec)
{
#ifdef DEBUG
#if DEBUG > 2
	printf("(%03hu) ParquetVideoDataF0: appending %u packets\n", id_, transport_stream_pkt_count);
#endif
#endif
	//SPDLOG_INFO("({:02d}) Writing VideoDataF0 to Parquet, {:d} rows", id_, temp_element_count_);

	for (int i = 0; i < transport_stream_pkt_count; i++)
	{
		doy_[temp_element_count_] = doy;
		ET_[temp_element_count_] = vid_flags->ET;
		IPH_[temp_element_count_] = vid_flags->IPH;
		KLV_[temp_element_count_] = vid_flags->KLV;
		PL_[temp_element_count_] = vid_flags->PL;
		SRS_[temp_element_count_] = vid_flags->SRS;

		if (vid_flags->IPH)
		{
			time_[temp_element_count_] = time_stamp[i];
		}
		else
		{
			time_[temp_element_count_] = time_stamp[0];
		}
		
		channel_id_[temp_element_count_] = channel_id;

		std::copy(data_vec.begin() + i * TransportStream_DATA_COUNT,
			data_vec.begin() + (i + 1) * TransportStream_DATA_COUNT, 
			video_data_.data() + temp_element_count_ * TransportStream_DATA_COUNT);

		// Increment the count variable.
		temp_element_count_++;

		if (temp_element_count_ == max_temp_element_count_)
		{
#ifdef DEBUG
#if DEBUG > 0
			printf("(%03u) Writing VideoDataF0 to Parquet, %d rows\n", id_, temp_element_count_);
#endif
#endif
			for (int i = 0; i < DEFAULT_BUFFER_SIZE_MULTIPLIER_VIDEO; i++)
			{
				WriteColumns(DEFAULT_ROW_GROUP_COUNT_VIDEO, i * DEFAULT_ROW_GROUP_COUNT_VIDEO);
			}

			temp_element_count_ = 0;
		}
	}
}

#ifdef PARSER_REWRITE

void ParquetVideoDataF0::append_data(
	const uint64_t& time_stamp, 
	const uint8_t& doy,
	const uint32_t& channel_id, 
	const Ch10VideoF0HeaderFormat& vid_flags, 
	const video_datum* const data)
{
	doy_[temp_element_count_] = doy;
	ET_[temp_element_count_] = vid_flags.ET;
	IPH_[temp_element_count_] = vid_flags.IPH;
	KLV_[temp_element_count_] = vid_flags.KLV;
	PL_[temp_element_count_] = vid_flags.PL;
	SRS_[temp_element_count_] = vid_flags.SRS;
	time_[temp_element_count_] = time_stamp;
	
	channel_id_[temp_element_count_] = channel_id;

	if (temp_element_count_ % 2500 == 0)  SPDLOG_DEBUG("Appending subpacket {:d} to video_data_", temp_element_count_);


	std::copy(data, data + TransportStream_DATA_COUNT, 
		video_data_.data() + temp_element_count_ * TransportStream_DATA_COUNT);

	// Increment the count variable.
	temp_element_count_++;

	if (temp_element_count_ == max_temp_element_count_)
	{
		SPDLOG_INFO("({:02d}) Writing VideoDataF0 to Parquet, {:d} rows", id_, temp_element_count_);

		for (int i = 0; i < DEFAULT_BUFFER_SIZE_MULTIPLIER_VIDEO; i++)
		{
			WriteColumns(DEFAULT_ROW_GROUP_COUNT_VIDEO, i * DEFAULT_ROW_GROUP_COUNT_VIDEO);
		}

		temp_element_count_ = 0;
	}
}

#endif // PARSER_REWRITE