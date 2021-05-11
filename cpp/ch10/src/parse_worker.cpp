// parse_worker.cpp

#include "parse_worker.h"

ParseWorker::~ParseWorker()
{
}

ParseWorker::ParseWorker() : start_position(0), 
	last_position(0), id(UINT16_MAX), complete(false),
	bb_ind(UINT16_MAX), final_worker(false)
{ }

void ParseWorker::initialize(uint16_t ID,
	uint64_t start_pos, uint32_t read, uint16_t binbuff_ind,
	std::map<Ch10PacketType, ManagedPath>& file_path_map,
	bool is_final_worker)
{
	id = ID;
	start_position = start_pos;
	bb_ind = binbuff_ind;
	final_worker = is_final_worker;
	output_file_paths_ = file_path_map;
}

void ParseWorker::append_mode_initialize(uint32_t read, uint16_t binbuff_ind,
	uint64_t start_pos)
{
	bb_ind = binbuff_ind;
	start_position = start_pos;
}

void ParseWorker::operator()(BinBuff& bb, bool append_mode, 
	std::vector<std::string>& tmats_body_vec, std::map<Ch10PacketType, bool> ch10_packet_type_map)
{
	if (append_mode)
		SPDLOG_INFO("({:02d}) APPEND MODE ParseWorker now active", id);
	else
		SPDLOG_INFO("({:02d}) ParseWorker now active", id);
	
	SPDLOG_DEBUG("({:02d}) Beginning of shift, absolute position: {:d}", id, start_position);

	// Initialize Ch10Context object. Note that this Ch10Context instance
	// created in the ParseWorker constructor is persistent until the 
	// ParseWorker instance is garbage-collected to maintain file writer
	// (read: Parquet file writer) state.
	ctx.Initialize(start_position, id);
	ctx.SetSearchingForTDP(!append_mode);

	if (!ctx.IsConfigured())
	{
		// Configure packet parsing.
		ctx.SetPacketTypeConfig(ch10_packet_type_map);

		// Configure output file paths.
		std::map<Ch10PacketType, ManagedPath> output_paths = {
			{Ch10PacketType::MILSTD1553_F1, output_file_paths_[Ch10PacketType::MILSTD1553_F1]},
			{Ch10PacketType::VIDEO_DATA_F0, output_file_paths_[Ch10PacketType::VIDEO_DATA_F0]}
		};

		// Check configuration. Are the packet parse and output paths configs
		// consistent?
		std::map<Ch10PacketType, ManagedPath> enabled_paths;
		bool config_ok = ctx.CheckConfiguration(ctx.pkt_type_config_map,
			output_paths, enabled_paths);
		if (!config_ok)
		{
			complete = true;
			return;
		}

		// For each packet type that is enabled and has an output path specified,
		// create a file writer object that is owned by Ch10Context to maintain
		// state between regular and append mode calls to this worker's operator().
		// Pass a pointer to the file writer to the relevant parser for use in 
		// writing data to disk.
		ctx.InitializeFileWriters(enabled_paths);
	}

	// Instantiate Ch10Packet object
	Ch10Packet packet(&bb, &ctx, tmats_body_vec);

	// Parse packets until error or end of buffer.
	bool continue_parsing = true;
	Ch10Status status;
	while (continue_parsing)
	{
		status = packet.ParseHeader();
		if (status == Ch10Status::BAD_SYNC || status == Ch10Status::PKT_TYPE_NO)
		{
			continue;
		}
		else if (status == Ch10Status::PKT_TYPE_EXIT || status == Ch10Status::BUFFER_LIMITED)
		{
			continue_parsing = false;
			continue;
		}

		// Parse body if the header is parsed and validated.
		packet.ParseBody();
	}

	// Update last_position;
	last_position = ctx.absolute_position;

	// Close all file writers if append_mode is true or
	// this is the final worker which has no append mode.
	if (append_mode || final_worker)
	{
		SPDLOG_DEBUG("({:02d}) Closing file writers", id);
		ctx.CloseFileWriters();
	}
	
	SPDLOG_INFO("({:02d}) End of worker's shift", id);
	SPDLOG_DEBUG("({:02d}) End of shift, absolute position: {:d}", id, last_position);
	complete = true;
}

std::atomic<bool>& ParseWorker::completion_status()
{
	return complete;
}

void ParseWorker::reset_completion_status()
{
	complete = false;
}

uint16_t ParseWorker::get_binbuff_ind()
{ return bb_ind; }

uint64_t& ParseWorker::get_last_position()
{
	return last_position;
}

void ParseWorker::append_chanid_remoteaddr_maps(std::map<uint32_t, std::set<uint16_t>>& out1,
	std::map<uint32_t, std::set<uint16_t>>&out2)
{
	IterableTools it;
	out1 = it.CombineCompoundMapsToSet(out1, ctx.chanid_remoteaddr1_map);
	out2 = it.CombineCompoundMapsToSet(out2, ctx.chanid_remoteaddr2_map);
}

void ParseWorker::append_chanid_comwmwords_map(std::map<uint32_t, std::set<uint32_t>>& out)
{
	IterableTools it;
	out = it.CombineCompoundMapsToSet(out, ctx.chanid_commwords_map);
}

const std::map<uint16_t, uint64_t>& ParseWorker::GetChannelIDToMinTimeStampMap()
{
	return ctx.chanid_minvideotimestamp_map;
}
