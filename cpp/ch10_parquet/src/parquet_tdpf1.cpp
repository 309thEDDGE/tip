#include "parquet_tdpf1.h"

const int ParquetTDPF1::TDP_ROW_GROUP_COUNT = 1000;
const int ParquetTDPF1::TDP_BUFFER_SIZE_MULTIPLIER = 1;

ParquetTDPF1::ParquetTDPF1(ParquetContext* pq_ctx) : pq_ctx_(pq_ctx),
                                            max_temp_element_count_(TDP_ROW_GROUP_COUNT * TDP_BUFFER_SIZE_MULTIPLIER),
                                            thread_id_(UINT16_MAX)
{
}

int ParquetTDPF1::GetRowGroupRowCount()
{
    return TDP_ROW_GROUP_COUNT;
}

int ParquetTDPF1::GetRowGroupBufferCount()
{
    return TDP_BUFFER_SIZE_MULTIPLIER;
}

int ParquetTDPF1::Initialize(const ManagedPath& outfile, uint16_t thread_id)
{
    thread_id_ = thread_id;
    outfile_ = outfile.string();

    // Allocate vector memory.
    time_stamp_.resize(max_temp_element_count_);
    src_.resize(max_temp_element_count_);
    time_fmt_.resize(max_temp_element_count_);
    leap_year_.resize(max_temp_element_count_);
    date_fmt_.resize(max_temp_element_count_);

    // Add fields to table.
    pq_ctx_->AddField(arrow::int64(), "time");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int8(), "src");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int8(), "time_fmt");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "leap_yr");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "date_fmt");  // GCOVR_EXCL_LINE

    // Set memory locations.
    pq_ctx_->SetMemoryLocation(time_stamp_, "time");
    pq_ctx_->SetMemoryLocation(src_, "src");
    pq_ctx_->SetMemoryLocation(time_fmt_, "time_fmt");
    pq_ctx_->SetMemoryLocation(leap_year_, "leap_yr");
    pq_ctx_->SetMemoryLocation(date_fmt_, "date_fmt");

    if(!pq_ctx_->OpenForWrite(outfile_, true))
    {
        SPDLOG_ERROR("({:03d}) OpenForWrite failed for file {:s}", thread_id_,
                     outfile_);
        return 74;
    }

    // Setup automatic tracking of appended data.
    if (!pq_ctx_->SetupRowCountTracking(TDP_ROW_GROUP_COUNT,
                               TDP_BUFFER_SIZE_MULTIPLIER, true, "TIME_DATA_F1"))
    {
        SPDLOG_ERROR("({:03d}) SetupRowCountTracking not configured correctly",
                     thread_id_);
        return 70;
    }

    pq_ctx_->EnableEmptyFileDeletion(outfile_);
    return 0;
}

void ParquetTDPF1::Append(const uint64_t& time_stamp, const TDF1CSDWFmt& tdp)
{
    time_stamp_[pq_ctx_->append_count_] = static_cast<int64_t>(time_stamp);
    src_[pq_ctx_->append_count_] = static_cast<int8_t>(tdp.src);
    time_fmt_[pq_ctx_->append_count_] = static_cast<int8_t>(tdp.time_fmt);
    leap_year_[pq_ctx_->append_count_] = static_cast<uint8_t>(tdp.leap_year);
    date_fmt_[pq_ctx_->append_count_] = static_cast<uint8_t>(tdp.date_fmt);

    pq_ctx_->IncrementAndWrite(thread_id_);
}
