#include "parquet_videodataf0.h"

const int ParquetVideoDataF0::DEFAULT_ROW_GROUP_COUNT_VIDEO = 10000;
const int ParquetVideoDataF0::DEFAULT_BUFFER_SIZE_MULTIPLIER_VIDEO = 10;

ParquetVideoDataF0::ParquetVideoDataF0(ParquetContext* parquet_context) : pq_ctx_(parquet_context),
                                           max_temp_element_count_(DEFAULT_ROW_GROUP_COUNT_VIDEO * DEFAULT_BUFFER_SIZE_MULTIPLIER_VIDEO),
                                           thread_id_(UINT16_MAX), outfile_("")
{
}

int ParquetVideoDataF0::GetRowGroupRowCount()
{
    return DEFAULT_ROW_GROUP_COUNT_VIDEO;
}

int ParquetVideoDataF0::GetRowGroupBufferCount()
{
    return DEFAULT_BUFFER_SIZE_MULTIPLIER_VIDEO;
}

int ParquetVideoDataF0::Initialize(ManagedPath outfile, uint16_t thread_id)
{
    thread_id_ = thread_id;
    outfile_ = outfile.string();

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
    pq_ctx_->AddField(arrow::boolean(), "doy");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "ET");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "IPH");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "KLV");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int16(), "PL");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "SRS");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int32(), "data", TransportStream_DATA_COUNT);  // GCOVR_EXCL_LINE  
    pq_ctx_->AddField(arrow::int64(), "time");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int32(), "channelid");  // GCOVR_EXCL_LINE

    // Set memory locations.
    pq_ctx_->SetMemoryLocation<uint8_t>(doy_, "doy");
    pq_ctx_->SetMemoryLocation<uint8_t>(ET_, "ET");
    pq_ctx_->SetMemoryLocation<uint8_t>(IPH_, "IPH");
    pq_ctx_->SetMemoryLocation<uint8_t>(KLV_, "KLV");
    pq_ctx_->SetMemoryLocation<int16_t>(PL_, "PL");
    pq_ctx_->SetMemoryLocation<uint8_t>(SRS_, "SRS");
    pq_ctx_->SetMemoryLocation<int32_t>(video_data_, "data");
    pq_ctx_->SetMemoryLocation<int64_t>(time_, "time");
    pq_ctx_->SetMemoryLocation<int32_t>(channel_id_, "channelid");

    if (!pq_ctx_->OpenForWrite(outfile_, true))
    {
        SPDLOG_ERROR("({:03d}) OpenForWrite failed for file {:s}", thread_id_,
                     outfile_);
        return 74;
    }

    // Setup automatic tracking of appended data.
    if (!pq_ctx_->SetupRowCountTracking(DEFAULT_ROW_GROUP_COUNT_VIDEO,
                               DEFAULT_BUFFER_SIZE_MULTIPLIER_VIDEO, true, "VideoDataF0"))
    {
        SPDLOG_ERROR("({:03d}) SetupRowCountTracking not configured correctly",
                     thread_id_);
        return 70;
    }

    pq_ctx_->EnableEmptyFileDeletion(outfile_);
    return 0;
}

void ParquetVideoDataF0::Append(
    const uint64_t& time_stamp,
    const uint8_t& doy,
    const uint32_t& channel_id,
    const Ch10VideoF0HeaderFormat& vid_flags,
    const video_datum* const data)
{
    doy_[pq_ctx_->append_count_] = doy;
    ET_[pq_ctx_->append_count_] = vid_flags.ET;
    IPH_[pq_ctx_->append_count_] = vid_flags.IPH;
    KLV_[pq_ctx_->append_count_] = vid_flags.KLV;
    PL_[pq_ctx_->append_count_] = static_cast<int16_t>(vid_flags.PL);
    SRS_[pq_ctx_->append_count_] = vid_flags.SRS;
    time_[pq_ctx_->append_count_] = static_cast<int64_t>(time_stamp);

    channel_id_[pq_ctx_->append_count_] = static_cast<int32_t>(channel_id);

    std::copy(data, data + TransportStream_DATA_COUNT,
              video_data_.data() + pq_ctx_->append_count_ * TransportStream_DATA_COUNT);

    // Increment the count variable and write data if row group(s) are filled.
    if (pq_ctx_->IncrementAndWrite(thread_id_))
    {
    }
}
