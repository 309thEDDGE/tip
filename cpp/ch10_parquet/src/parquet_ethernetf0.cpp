#include "parquet_ethernetf0.h"

const size_t ParquetEthernetF0::DEFAULT_ROW_GROUP_COUNT = 10000;
const size_t ParquetEthernetF0::DEFAULT_BUFFER_SIZE_MULTIPLIER = 10;
const size_t ParquetEthernetF0::PAYLOAD_LIST_COUNT = static_cast<size_t>(
    EthernetData::max_payload_size_);
const size_t ParquetEthernetF0::MAX_TEMP_ELEMENT_COUNT = 
    ParquetEthernetF0::DEFAULT_ROW_GROUP_COUNT * ParquetEthernetF0::DEFAULT_BUFFER_SIZE_MULTIPLIER;

ParquetEthernetF0::ParquetEthernetF0(ParquetContext* pq_ctx) : pq_ctx_(pq_ctx),
                                         thread_id_(UINT16_MAX),
                                         payload_ptr_(nullptr), outfile_("")
{
}

int ParquetEthernetF0::GetRowGroupRowCount()
{
    return DEFAULT_ROW_GROUP_COUNT;
}

int ParquetEthernetF0::GetRowGroupBufferCount()
{
    return DEFAULT_BUFFER_SIZE_MULTIPLIER;
}

int ParquetEthernetF0::GetDataPayloadListElementCount()
{
    return PAYLOAD_LIST_COUNT;
}

bool ParquetEthernetF0::Initialize(const ManagedPath& outfile, uint16_t thread_id)
{
    thread_id_ = thread_id;
    outfile_ = outfile.string();

    // Allocate vector memory.
    time_stamp_.resize(MAX_TEMP_ELEMENT_COUNT);
    channel_id_.resize(MAX_TEMP_ELEMENT_COUNT);
    payload_.resize(MAX_TEMP_ELEMENT_COUNT * PAYLOAD_LIST_COUNT, 0);
    payload_ptr_ = payload_.data();
    payload_size_.resize(MAX_TEMP_ELEMENT_COUNT);
    dst_mac_addr_.resize(MAX_TEMP_ELEMENT_COUNT);
    src_mac_addr_.resize(MAX_TEMP_ELEMENT_COUNT);
    ethertype_.resize(MAX_TEMP_ELEMENT_COUNT);
    frame_format_.resize(MAX_TEMP_ELEMENT_COUNT);
    dsap_.resize(MAX_TEMP_ELEMENT_COUNT);
    ssap_.resize(MAX_TEMP_ELEMENT_COUNT);
    snd_seq_number_.resize(MAX_TEMP_ELEMENT_COUNT);
    rcv_seq_number_.resize(MAX_TEMP_ELEMENT_COUNT);
    dst_ip_addr_.resize(MAX_TEMP_ELEMENT_COUNT);
    src_ip_addr_.resize(MAX_TEMP_ELEMENT_COUNT);
    id_.resize(MAX_TEMP_ELEMENT_COUNT);
    protocol_.resize(MAX_TEMP_ELEMENT_COUNT);
    offset_.resize(MAX_TEMP_ELEMENT_COUNT);
    dst_port_.resize(MAX_TEMP_ELEMENT_COUNT);
    src_port_.resize(MAX_TEMP_ELEMENT_COUNT);

    // Add fields to table.
    pq_ctx_->AddField(arrow::int64(), "time");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int32(), "channelid");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int16(), "payload", PAYLOAD_LIST_COUNT);  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int64(), "payload_sz");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::utf8(), "dstmac");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::utf8(), "srcmac");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int32(), "ethtype");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int16(), "llcfmt");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int16(), "llcdsap");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int16(), "llcssap");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int16(), "llcsndseqnum");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int16(), "llcrcvseqnum");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::utf8(), "dstip");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::utf8(), "srcip");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int32(), "ipid");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int16(), "ipproto");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int32(), "ipoffset");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int32(), "dstport");  // GCOVR_EXCL_LINE
    pq_ctx_->AddField(arrow::int32(), "srcport");  // GCOVR_EXCL_LINE

    // Set memory locations.
    pq_ctx_->SetMemoryLocation(time_stamp_, "time");
    pq_ctx_->SetMemoryLocation(channel_id_, "channelid");
    pq_ctx_->SetMemoryLocation(payload_, "payload");
    pq_ctx_->SetMemoryLocation(payload_size_, "payload_sz");
    pq_ctx_->SetMemoryLocation(dst_mac_addr_, "dstmac");
    pq_ctx_->SetMemoryLocation(src_mac_addr_, "srcmac");
    pq_ctx_->SetMemoryLocation(ethertype_, "ethtype");
    pq_ctx_->SetMemoryLocation(frame_format_, "llcfmt");
    pq_ctx_->SetMemoryLocation(dsap_, "llcdsap");
    pq_ctx_->SetMemoryLocation(ssap_, "llcssap");
    pq_ctx_->SetMemoryLocation(snd_seq_number_, "llcsndseqnum");
    pq_ctx_->SetMemoryLocation(rcv_seq_number_, "llcrcvseqnum");
    pq_ctx_->SetMemoryLocation(dst_ip_addr_, "dstip");
    pq_ctx_->SetMemoryLocation(src_ip_addr_, "srcip");
    pq_ctx_->SetMemoryLocation(id_, "ipid");
    pq_ctx_->SetMemoryLocation(protocol_, "ipproto");
    pq_ctx_->SetMemoryLocation(offset_, "ipoffset");
    pq_ctx_->SetMemoryLocation(dst_port_, "dstport");
    pq_ctx_->SetMemoryLocation(src_port_, "srcport");

    if (!pq_ctx_->OpenForWrite(outfile_, true))
    {
        SPDLOG_ERROR("({:03d}) OpenForWrite failed for file {:s}", thread_id_,
                     outfile_);
        return false;
    }

    // Setup automatic tracking of appended data.
    if (!pq_ctx_->SetupRowCountTracking(DEFAULT_ROW_GROUP_COUNT,
                               DEFAULT_BUFFER_SIZE_MULTIPLIER, true, "EthernetF0"))
    {
        SPDLOG_ERROR("({:03d}) SetupRowCountTracking not configured correctly",
                     thread_id_);
        return false;
    }

    pq_ctx_->EnableEmptyFileDeletion(outfile_);
    return true;
}

void ParquetEthernetF0::Append(const uint64_t& time_stamp, const uint32_t& chanid,
                               const EthernetData* eth_data)
{
    //printf("pq_ctx_->append_count_ is %zu\n", pq_ctx_->append_count_);
    time_stamp_[pq_ctx_->append_count_] = static_cast<int64_t>(time_stamp);
    channel_id_[pq_ctx_->append_count_] = static_cast<int32_t>(chanid);
    payload_size_[pq_ctx_->append_count_] = static_cast<int64_t>(eth_data->payload_size_);
    dst_mac_addr_[pq_ctx_->append_count_] = eth_data->dst_mac_addr_;
    src_mac_addr_[pq_ctx_->append_count_] = eth_data->src_mac_addr_;
    ethertype_[pq_ctx_->append_count_] = static_cast<int32_t>(eth_data->ethertype_);
    frame_format_[pq_ctx_->append_count_] = static_cast<int16_t>(eth_data->frame_format_);
    dsap_[pq_ctx_->append_count_] = static_cast<int16_t>(eth_data->dsap_);
    ssap_[pq_ctx_->append_count_] = static_cast<int16_t>(eth_data->ssap_);
    snd_seq_number_[pq_ctx_->append_count_] = static_cast<int16_t>(eth_data->snd_seq_number_);
    rcv_seq_number_[pq_ctx_->append_count_] = static_cast<int16_t>(eth_data->rcv_seq_number_);
    dst_ip_addr_[pq_ctx_->append_count_] = eth_data->dst_ip_addr_;
    src_ip_addr_[pq_ctx_->append_count_] = eth_data->src_ip_addr_;
    id_[pq_ctx_->append_count_] = static_cast<int32_t>(eth_data->id_);
    protocol_[pq_ctx_->append_count_] = static_cast<int16_t>(eth_data->protocol_);
    offset_[pq_ctx_->append_count_] = static_cast<int32_t>(eth_data->offset_);
    dst_port_[pq_ctx_->append_count_] = static_cast<int32_t>(eth_data->dst_port_);
    src_port_[pq_ctx_->append_count_] = static_cast<int32_t>(eth_data->src_port_);

    // Copy payload
    std::copy(eth_data->payload_ptr_, eth_data->payload_ptr_ + eth_data->payload_size_,
              payload_ptr_ + pq_ctx_->append_count_ * PAYLOAD_LIST_COUNT);

    // Increment the count variable and write data if row group(s) are filled.
    if (pq_ctx_->IncrementAndWrite(thread_id_))
    {
        // Reset list buffers.
        std::fill(payload_.begin(), payload_.end(), 0);
    }
}