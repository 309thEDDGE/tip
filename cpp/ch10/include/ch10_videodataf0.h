#ifndef CH10VIDEODATAF0_H
#define CH10VIDEODATAF0_H

#include "parse_context.h"
#include "VideoDataF0Format.h"
#include "tmats.h"

#ifdef LOCALDB
#ifdef PARQUET
#include "parquet_videodataf0.h"
#endif
#endif 

#ifdef LIBIRIG106
extern "C" {
#include "i106_decode_video.h"
}
#endif

class Ch10VideoDataF0 : public ParseContext<VideoDataF0ChanSpecFormat, VideoDataF1Status>
{
private: 
#ifdef LIBIRIG106
	I106Status i106_status_;
	VideoF0_Message i106_videomsg_;
#endif
	//TMATS& tdata;

	// Map of channel id to label if given in the TMATS preface.
	// This may be an empty map if there are no TMATS data from
	// which the map can be created.
	std::map<uint32_t, std::string> chanid_to_label_map;

	// Indicates if the map contains data.
	bool have_chanid_to_label_map;

	// Temporary string to which a label can be assigned in the
	// absence of a TMATS-defined label.
	std::string chanid_label;

	// Pointer which is assigned to the position in the Ch10 which
	// corresponds to the beginning of transport stream packet.
	const video_datum* transport_stream_pkt;

	// Vector of transport stream data in chunks of 'video_datum' type.
	std::vector<video_datum> transport_stream_data;

	// Iterator to transport_stream_data vector.
	std::vector<video_datum>::iterator video_datum_it;

	// Transport stream time stamps, if present in the Video packet.
	std::vector<uint64_t> transport_stream_TS;

	// Number of video_datum units to be copied for each transport
	// stream subpkt. The sub-packet terminology is used to identify
	// a 188-byte transport stream packet in a larger Ch10 video packet. 
	// E.g., 10 subpkts = quantity 10 188-byte packets with time stamp
	// information if specified by the Ch10 header.
	//const uint8_t transport_stream_copy_size; --> use TransportStream_DATA_SIZE instead

	std::string outpath;

	// Total size of packet body in bytes including channel specific data and possible 
	// intra-pkt time stamps in addition to multiple 188-byte transport stream packets.
	//uint32_t data_size;

	// Size of subpkt in bytes. Either 188-byte transport stream packet or
	// 188-byte transport stream packet plus the time stamp, if indicated by
	// the Ch10 metadata.
	uint32_t subpkt_unit_size;

	// Quantity of subpkts in the current Ch10 video packet.
	uint32_t subpkt_unit_count;

	// Subpkt index within the Ch10 video data packet.
	uint32_t subpkt_unit_index;

	// Quantity of video_datum units across all subpkts in the Ch10 video packet,
	// not including time information, if present. Equal to subpkt_unit_count multiplied
	// by the number of video_datum units in a subpkt. The latter is defined as
	// transport_stream_copy_size.
	uint32_t total_data_unit_count;

	void print_video_pkt_info();

	std::map<uint16_t, uint64_t> channel_id_to_min_time_map_;

	// If there is no entry in channel_id_to_min_time_map_ for the
	// current channel ID then add it and the earliest time stamp in
	// the packet to the map. Otherwise, replace the entry only if 
	// the current time stamp is lower.
	void RecordLowestTimeStampPerChannelID();

#ifdef LOCALDB
#ifdef PARQUET
	ParquetVideoDataF0 db;
#endif
#endif

public:
	~Ch10VideoDataF0();
	Ch10VideoDataF0(BinBuff& buff, uint16_t ID, TMATS& tmats, std::string out_path);
	void Initialize(const Ch10TimeData* ch10td, const Ch10HeaderData* ch10hd) override;
	uint8_t Parse() override;
	void close();
	void set_truncate(bool state);
#ifdef LIBIRIG106
	uint8_t UseLibIRIG106(I106C10Header* i106_header, void* buffer);
	uint8_t IngestLibIRIG106Msg();
	const std::map<uint16_t, uint64_t>& GetChannelIDToMinTimeStampMap();
#endif
};

#endif