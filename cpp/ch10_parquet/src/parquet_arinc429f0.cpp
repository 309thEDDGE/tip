#include "parquet_arinc429f0.h"

ParquetARINC429F0::ParquetARINC429F0() : max_temp_element_count_(ARINC429_ROW_GROUP_COUNT * ARINC429_BUFFER_SIZE_MULTIPLIER),
                                             ParquetContext(ARINC429_ROW_GROUP_COUNT),
                                             thread_id_(UINT16_MAX)
{
}

bool ParquetARINC429F0::Initialize(const ManagedPath& outfile, uint16_t thread_id)
{
    thread_id_ = thread_id;

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
    AddField(arrow::int64(), "time");
    AddField(arrow::int16(), "doy");
    AddField(arrow::int32(), "channelid");
    AddField(arrow::int32(), "gaptime");
    AddField(arrow::boolean(), "BS");
    AddField(arrow::boolean(), "PE");
    AddField(arrow::boolean(), "FE");
    AddField(arrow::int16(), "bus");
    AddField(arrow::int16(), "label");
    AddField(arrow::int8(), "SDI");
    AddField(arrow::int32(), "data");
    AddField(arrow::int8(), "SSM");
    AddField(arrow::boolean(), "parity");

    // Set memory locations.
    SetMemoryLocation(time_stamp_, "time");
    SetMemoryLocation(doy_, "doy");
    SetMemoryLocation(gap_time_, "gaptime");
    SetMemoryLocation(BS_, "BS");
    SetMemoryLocation(PE_, "PE");
    SetMemoryLocation(FE_, "FE");
    SetMemoryLocation(bus_, "bus");
    SetMemoryLocation(label_, "label");
    SetMemoryLocation(SDI_, "SDI");
    SetMemoryLocation(data_, "data");
    SetMemoryLocation(SSM_, "SSM");
    SetMemoryLocation(parity_, "parity");
    SetMemoryLocation(channel_id_, "channelid");

    if(!OpenForWrite(outfile.string(), true))// set correct field from here down
    {
        SPDLOG_ERROR("({:03d}) OpenForWrite failed for file {:s}", thread_id_,
                     outfile.string());
        return false;
    }

    // Setup automatic tracking of appended data.
    if (!SetupRowCountTracking(ARINC429_ROW_GROUP_COUNT,
                               ARINC429_BUFFER_SIZE_MULTIPLIER, true, "ARINC429F0"))
    {
        SPDLOG_ERROR("({:03d}) SetupRowCountTracking not configured correctly",
                     thread_id_);
        return false;
    }

    EnableEmptyFileDeletion(outfile.string());
    return true;
}

void ParquetARINC429F0::Append(const uint64_t& time_stamp, uint8_t doy,
                const ARINC429F0CSDWFmt* const chan_spec,
                const ARINC429F0MsgFmt* msg, const uint16_t& chanid)
{
    // Is the csdw even needed? Don't see how it is for ARINC data.

    time_stamp_[append_count_] = time_stamp;
    doy_[append_count_] = doy;
    gap_time_[append_count_] = msg->gap;
    BS_[append_count_] = msg->BS;
    PE_[append_count_] = msg->PE;
    FE_[append_count_] = msg->FE;
    bus_[append_count_] = msg->bus;
    label_[append_count_] = EncodeARINC429Label(msg->label);
    SDI_[append_count_] = msg->SDI;
    data_[append_count_] = msg->data;
    SSM_[append_count_] = msg->SSM;
    parity_[append_count_] = msg->parity;
    channel_id_[append_count_] = chanid;

    // Increment the count variable and write data if row group(s) are filled.
    if (IncrementAndWrite(thread_id_))
    {
        // Reset list buffers.
        std::fill(data_.begin(), data_.end(), 0);
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