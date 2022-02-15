#include "parquet_arinc429f0.h"

const int ParquetARINC429F0::ARINC429_ROW_GROUP_COUNT = 10000;
const int ParquetARINC429F0::ARINC429_BUFFER_SIZE_MULTIPLIER = 10;

ParquetARINC429F0::ParquetARINC429F0(ParquetContext* pq_ctx) : pq_ctx_(pq_ctx),
                                            max_temp_element_count_(ARINC429_ROW_GROUP_COUNT * ARINC429_BUFFER_SIZE_MULTIPLIER),
                                            thread_id_(UINT16_MAX)
{
}

bool ParquetARINC429F0::Initialize(const ManagedPath& outfile, uint16_t thread_id)
{
    thread_id_ = thread_id;
    outfile_ = outfile.string();

    // Allocate vector memory.
    time_stamp_.resize(max_temp_element_count_);
    doy_.resize(max_temp_element_count_);
    gap_time_.resize(max_temp_element_count_);
    BS_.resize(max_temp_element_count_);
    PE_.resize(max_temp_element_count_);
    FE_.resize(max_temp_element_count_);
    bus_.resize(max_temp_element_count_);
    label_.resize(max_temp_element_count_);
    SDI_.resize(max_temp_element_count_);
    data_.resize(max_temp_element_count_);
    SSM_.resize(max_temp_element_count_);
    parity_.resize(max_temp_element_count_);
    channel_id_.resize(max_temp_element_count_);

    // Add fields to table.
    pq_ctx_->AddField(arrow::int64(), "time");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "doy");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int32(), "channelid");  // LCOV_EXCL_LINE
    pq_ctx_->AddField(arrow::int32(), "gaptime");  // GCOV_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "BS");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "PE");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "FE");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int16(), "bus");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int16(), "label");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int8(), "SDI");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int32(), "data");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int8(), "SSM");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::boolean(), "parity");  // GCOVR_EXCL_LINE

    // Set memory locations.
    pq_ctx_->SetMemoryLocation(time_stamp_, "time");
    pq_ctx_->SetMemoryLocation(doy_, "doy");
    pq_ctx_->SetMemoryLocation(gap_time_, "gaptime");
    pq_ctx_->SetMemoryLocation(BS_, "BS");
    pq_ctx_->SetMemoryLocation(PE_, "PE");
    pq_ctx_->SetMemoryLocation(FE_, "FE");
    pq_ctx_->SetMemoryLocation(bus_, "bus");
    pq_ctx_->SetMemoryLocation(label_, "label");
    pq_ctx_->SetMemoryLocation(SDI_, "SDI");
    pq_ctx_->SetMemoryLocation(data_, "data");
    pq_ctx_->SetMemoryLocation(SSM_, "SSM");
    pq_ctx_->SetMemoryLocation(parity_, "parity");
    pq_ctx_->SetMemoryLocation(channel_id_, "channelid");

    if(!pq_ctx_->OpenForWrite(outfile_, true))
    {
        SPDLOG_ERROR("({:03d}) OpenForWrite failed for file {:s}", thread_id_,
                     outfile_);
        return false;
    }

    // Setup automatic tracking of appended data.
    if (!pq_ctx_->SetupRowCountTracking(ARINC429_ROW_GROUP_COUNT,
                               ARINC429_BUFFER_SIZE_MULTIPLIER, true, "ARINC429F0"))
    {
        SPDLOG_ERROR("({:03d}) SetupRowCountTracking not configured correctly",
                     thread_id_);
        return false;
    }

    pq_ctx_->EnableEmptyFileDeletion(outfile_);
    return true;
}

void ParquetARINC429F0::Append(const uint64_t& time_stamp, uint8_t doy,
                const ARINC429F0MsgFmt* msg, const uint16_t& chanid)
{
    time_stamp_[pq_ctx_->append_count_] = static_cast<int64_t>(time_stamp);
    doy_[pq_ctx_->append_count_] = doy;
    gap_time_[pq_ctx_->append_count_] = static_cast<int32_t>(msg->gap);
    BS_[pq_ctx_->append_count_] = msg->BS;
    PE_[pq_ctx_->append_count_] = msg->PE;
    FE_[pq_ctx_->append_count_] = msg->FE;
    bus_[pq_ctx_->append_count_] = static_cast<int16_t>(msg->bus);
    label_[pq_ctx_->append_count_] = static_cast<int16_t>(msg->label);
    SDI_[pq_ctx_->append_count_] = static_cast<int8_t>(msg->SDI);
    data_[pq_ctx_->append_count_] = static_cast<int32_t>(msg->data);
    SSM_[pq_ctx_->append_count_] = static_cast<int8_t>(msg->SSM);
    parity_[pq_ctx_->append_count_] = msg->parity;
    channel_id_[pq_ctx_->append_count_] = static_cast<int32_t>(chanid);

    // Increment the count variable and write data if row group(s) are filled.
    if (pq_ctx_->IncrementAndWrite(thread_id_))
    {
    }

}

uint32_t ParquetARINC429F0::EncodeARINC429Label(uint32_t raw_label)
{
    // reverse bits in label
    raw_label = (raw_label & 0xF0) >> 4 | (raw_label & 0x0F) << 4;
    raw_label = (raw_label & 0xCC) >> 2 | (raw_label & 0x33) << 2;
    raw_label = (raw_label & 0xAA) >> 1 | (raw_label & 0x55) << 1;

    uint32_t octal_label = 0;
    octal_label = octal_label + ((raw_label >> 6) & 3)*((uint16_t)100);
    octal_label = octal_label + ((raw_label >> 3) & 7)*((uint16_t)10);
    octal_label = octal_label + (raw_label & 7);

    return octal_label;
}
