#ifndef PARQUET_ETHERNETF0_H
#define PARQUET_ETHERNETF0_H

#include <cstdint>
#include <string>
#include <vector>
#include "parquet_context.h"
#include "ethernet_data.h"
#include "managed_path.h"
#include "spdlog/spdlog.h"

class ParquetEthernetF0 : public ParquetContext
{
private:
	const size_t DEFAULT_ROW_GROUP_COUNT = 10000;
	const size_t DEFAULT_BUFFER_SIZE_MULTIPLIER = 10;
	const size_t PAYLOAD_LIST_COUNT;
	const size_t MAX_TEMP_ELEMENT_COUNT;
	uint16_t thread_id_;
	uint8_t* payload_ptr_;

	// Arrays of data to be written to the Parquet table. See EthernetData for a 
	// description of the columns.
	std::vector<uint64_t> time_stamp_;  // save as int64
	std::vector<uint8_t> payload_; // save as int16
	std::vector<int64_t> payload_size_; // original type is uint32_t
	std::vector<std::string> dst_mac_addr_;
	std::vector<std::string> src_mac_addr_;
	std::vector<int32_t> ethertype_; // original type is uint16_t
	std::vector<int16_t> frame_format_; // original is uint8_t
	std::vector<int16_t> dsap_; // original is uint8_t
	std::vector<int16_t> ssap_; // original is uint8_t
	std::vector<int16_t> snd_seq_number_; // original is uint8_t
	std::vector<int16_t> rcv_seq_number_; // original is uint8_t
	std::vector<std::string> dst_ip_addr_;
	std::vector<std::string> src_ip_addr_;
	std::vector<int32_t> id_; // original type is uint16_t
	std::vector<int16_t> protocol_; // original is uint8_t
	std::vector<int32_t> offset_; // original type is uint16_t
	std::vector<int32_t> dst_port_; // original type is uint16_t
	std::vector<int32_t> src_port_; // original type is uint16_t

public:
	ParquetEthernetF0();
	bool Initialize(const ManagedPath& outfile, uint16_t thread_id);
	void Append(const uint64_t& time_stamp, const EthernetData* eth_data);
};

#endif