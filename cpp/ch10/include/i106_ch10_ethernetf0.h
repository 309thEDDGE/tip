
#ifndef I106_CH10_ETHERNETF0_H
#define I106_CH10_ETHERNETF0_H

#include <sstream>
#include "i106_parse_context.h"

// Must include parquet_ethernetf0.h prior to network_packet_parser.h
#include "parquet_ethernetf0.h"
#include "network_packet_parser.h"
extern "C" {
#include "i106_decode_ethernet.h"
}

class I106Ch10EthernetF0 : public I106ParseContext
{
private:
	ParquetEthernetF0 pq_eth_writer_;
	I106Status i106_status_;
	EthernetF0_Message i106_ethmsg_;
	const EthernetF0_IPH* i106_ethiph_;
	const EthernetF0_Physical_FullMAC* i106_ethframe_;
	uint32_t frame_index_;
	uint16_t framelen_;
	uint8_t* framelen_ptr_;
	const uint8_t* typelen_ptr_;
	std::string dest_mac_addr_;
	std::string src_mac_addr_;
	std::stringstream dest_mac_stream_;
	std::stringstream src_mac_stream_;

	NetworkPacketParser npp;

public:
	const uint16_t& frame_length_ = framelen_;
	I106Ch10EthernetF0();
	bool InitializeWriter() override;
	uint8_t Ingest(I106C10Header* header, void* buffer);
	uint8_t RecordFrame();
	void CalculateFrameLength();
	void CreateStringMACAddrs();
	void Finalize();
};

#endif
