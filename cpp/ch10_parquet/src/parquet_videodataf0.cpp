#include "parquet_videodataf0.h"

ParquetVideoDataF0::ParquetVideoDataF0() : max_temp_element_count_(DEFAULT_ROW_GROUP_COUNT_VIDEO * DEFAULT_BUFFER_SIZE_MULTIPLIER_VIDEO),
                                           ParquetContext(DEFAULT_ROW_GROUP_COUNT_VIDEO),
                                           thread_id_(UINT16_MAX)
{
}

bool ParquetVideoDataF0::Initialize(ManagedPath outfile, uint16_t thread_id)
{
    thread_id_ = thread_id;

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

    if (!OpenForWrite(outfile.string(), true))
    {
        SPDLOG_ERROR("({:03d}) OpenForWrite failed for file {:s}", thread_id_,
                     outfile.string());
        return false;
    }

    // Setup automatic tracking of appended data.
    if (!SetupRowCountTracking(DEFAULT_ROW_GROUP_COUNT_VIDEO,
                               DEFAULT_BUFFER_SIZE_MULTIPLIER_VIDEO, true, "VideoDataF0"))
    {
        SPDLOG_ERROR("({:03d}) SetupRowCountTracking not configured correctly",
                     thread_id_);
        return false;
    }

    EnableEmptyFileDeletion(outfile.string());
    return true;
}

void ParquetVideoDataF0::Append(
    const uint64_t& time_stamp,
    const uint8_t& doy,
    const uint32_t& channel_id,
    const Ch10VideoF0HeaderFormat& vid_flags,
    const video_datum* const data)
{
    doy_[append_count_] = doy;
    ET_[append_count_] = vid_flags.ET;
    IPH_[append_count_] = vid_flags.IPH;
    KLV_[append_count_] = vid_flags.KLV;
    PL_[append_count_] = vid_flags.PL;
    SRS_[append_count_] = vid_flags.SRS;
    time_[append_count_] = time_stamp;

    channel_id_[append_count_] = channel_id;

    std::copy(data, data + TransportStream_DATA_COUNT,
              video_data_.data() + append_count_ * TransportStream_DATA_COUNT);

    // Increment the count variable and write data if row group(s) are filled.
    if (IncrementAndWrite(thread_id_))
    {
    }
}
