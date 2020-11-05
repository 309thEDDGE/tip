#include "parquet_ethernetf0.h"

ParquetEthernetF0::ParquetEthernetF0() : ParquetContext(DEFAULT_ROW_GROUP_COUNT),
thread_id_(UINT16_MAX), PAYLOAD_LIST_COUNT(EthernetData::mtu_), 
MAX_TEMP_ELEMENT_COUNT(DEFAULT_ROW_GROUP_COUNT* DEFAULT_BUFFER_SIZE_MULTIPLIER),
payload_ptr_(nullptr)
{

}

bool ParquetEthernetF0::Initialize(const std::string& outfile, uint16_t thread_id)
{
	thread_id_ = thread_id;

	// Allocate vector memory. 
	time_stamp_.resize(MAX_TEMP_ELEMENT_COUNT);
	payload_.resize(MAX_TEMP_ELEMENT_COUNT * PAYLOAD_LIST_COUNT, 0);
	payload_ptr_ = payload_.data();

	payload_size_.resize(MAX_TEMP_ELEMENT_COUNT);
	dst_mac_addr_.resize(MAX_TEMP_ELEMENT_COUNT);
	src_mac_addr_.resize(MAX_TEMP_ELEMENT_COUNT);
	payload_type_.resize(MAX_TEMP_ELEMENT_COUNT);
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
	AddField(arrow::int64(), "time");
	AddField(arrow::int16(), "payload", PAYLOAD_LIST_COUNT);
	AddField(arrow::int64(), "payload_sz");
	AddField(arrow::utf8(), "dstmac");
	AddField(arrow::utf8(), "srcmac");
	AddField(arrow::int32(), "ethtype");
	AddField(arrow::int16(), "llcfmt");
	AddField(arrow::int16(), "llcdsap");
	AddField(arrow::int16(), "llcssap");
	AddField(arrow::int16(), "llcsndseqnum");
	AddField(arrow::int16(), "llcrcvseqnum");
	AddField(arrow::utf8(), "dstip");
	AddField(arrow::utf8(), "srcip");
	AddField(arrow::int32(), "ipid");
	AddField(arrow::int16(), "ipproto");
	AddField(arrow::int32(), "ipoffset");
	AddField(arrow::int32(), "ipdstport");
	AddField(arrow::int32(), "ipsrcport");

	// Set memory locations.
	SetMemoryLocation(time_stamp_, "time");
	SetMemoryLocation(payload_, "payload");
	SetMemoryLocation(payload_size_, "payload_sz");
	SetMemoryLocation(dst_mac_addr_, "dstmac");
	SetMemoryLocation(src_mac_addr_, "srcmac");
	SetMemoryLocation(payload_type_, "ethtype");
	SetMemoryLocation(frame_format_, "llcfmt");
	SetMemoryLocation(dsap_, "llcdsap");
	SetMemoryLocation(ssap_, "llcssap");
	SetMemoryLocation(snd_seq_number_, "llcsndseqnum");
	SetMemoryLocation(rcv_seq_number_, "llcrcvseqnum");
	SetMemoryLocation(dst_ip_addr_, "dstip");
	SetMemoryLocation(src_ip_addr_, "srcip");
	SetMemoryLocation(id_, "ipid");
	SetMemoryLocation(protocol_, "ipproto");
	SetMemoryLocation(offset_, "ipoffset");
	SetMemoryLocation(dst_port_, "ipdstport");
	SetMemoryLocation(src_port_, "ipsrcport");

	if (!OpenForWrite(outfile, true))
	{
		printf("(%03hu) ParquetEthernetF0::Initialize(): OpenForWrite failed!\n",
			thread_id_);
		return false;
	}

	// Setup automatic tracking of appended data.
	char buff[100];
	sprintf(buff, "(%03hu) EthernetF0", thread_id_);
	std::string msg(buff);
	if (!SetupRowCountTracking(DEFAULT_ROW_GROUP_COUNT, DEFAULT_BUFFER_SIZE_MULTIPLIER, true, msg))
	{
		printf("(%03hu) ParquetEthernetF0::Initialize(): Row count tracking not configured correctly!\n",
			thread_id_);
		return false;
	}

	return true;
}

void ParquetEthernetF0::Append(const uint64_t& time_stamp, const EthernetData* eth_data)
{
	//printf("append_count_ is %zu\n", append_count_);
	time_stamp_[append_count_] = time_stamp;
	payload_size_[append_count_] = eth_data->payload_size_;
	dst_mac_addr_[append_count_] = eth_data->dst_mac_addr_;
	src_mac_addr_[append_count_] = eth_data->src_mac_addr_;
	payload_type_[append_count_] = eth_data->payload_type_;
	frame_format_[append_count_] = eth_data->frame_format_;
	dsap_[append_count_] = eth_data->dsap_;
	ssap_[append_count_] = eth_data->ssap_;
	snd_seq_number_[append_count_] = eth_data->snd_seq_number_;
	rcv_seq_number_[append_count_] = eth_data->rcv_seq_number_;
	dst_ip_addr_[append_count_] = eth_data->dst_ip_addr_;
	src_ip_addr_[append_count_] = eth_data->src_ip_addr_;
	id_[append_count_] = eth_data->id_;
	protocol_[append_count_] = eth_data->protocol_;
	offset_[append_count_] = eth_data->offset_;
	dst_port_[append_count_] = eth_data->dst_port_;
	src_port_[append_count_] = eth_data->src_port_;

	// Copy payload
	std::copy(eth_data->payload_ptr_, eth_data->payload_ptr_ + eth_data->payload_size_,
		payload_ptr_ + append_count_ * PAYLOAD_LIST_COUNT);

	// Increment the count variable and write data if row group(s) are filled.
	if (IncrementAndWrite())
	{
		// Reset list buffers.
		std::fill(payload_.begin(), payload_.end(), 0);
	}
}