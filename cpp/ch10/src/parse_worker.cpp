// parse_worker.cpp
#include "parse_worker.h"

ParseWorker::ParseWorker() : complete_(false)
{ }

//void ParseWorker::initialize(uint16_t worker_index,
//	uint64_t start_pos, uint32_t read, uint16_t buffer_index,
//	std::map<Ch10PacketType, ManagedPath>& file_path_map,
//	bool is_final_worker)
//{
//	worker_index_ = worker_index;
//	start_position_ = start_pos;
//	buffer_index_ = buffer_index;
//	final_worker_ = is_final_worker;
//	output_file_paths_ = file_path_map;
//}
//
//void ParseWorker::append_mode_initialize(uint32_t read, uint16_t buffer_index,
//	uint64_t start_pos)
//{
//	buffer_index_ = buffer_index;
//	start_position_ = start_pos;
//}

void ParseWorker::operator()(WorkerConfig& worker_config, 
	std::vector<std::string>& tmats_body_vec)
{
	if (worker_config.append_mode_)
		SPDLOG_INFO("({:02d}) APPEND MODE ParseWorker now active", worker_config.worker_index_);
	else
		SPDLOG_INFO("({:02d}) ParseWorker now active", worker_config.worker_index_);
	
	SPDLOG_DEBUG("({:02d}) Beginning of shift, absolute position: {:d}", 
		worker_config.worker_index_, worker_config.start_position_);

	// Initialize Ch10Context object. Note that this Ch10Context instance
	// created in the ParseWorker constructor is persistent until the 
	// ParseWorker instance is garbage-collected to maintain file writer
	// (read: Parquet file writer) state.
	ctx_.Initialize(worker_config.start_position_, worker_config.worker_index_);
	ctx_.SetSearchingForTDP(!worker_config.append_mode_);

	if (!ctx_.IsConfigured())
	{
		// Configure packet parsing.
		ctx_.SetPacketTypeConfig(worker_config.ch10_packet_type_map_);

		// Configure output file paths.
		/*std::map<Ch10PacketType, ManagedPath> output_paths = {
			{Ch10PacketType::MILSTD1553_F1, output_file_paths_[Ch10PacketType::MILSTD1553_F1]},
			{Ch10PacketType::VIDEO_DATA_F0, output_file_paths_[Ch10PacketType::VIDEO_DATA_F0]}
		};*/

		// Check configuration. Are the packet parse and output paths configs
		// consistent?
		std::map<Ch10PacketType, ManagedPath> enabled_paths;
		bool config_ok = ctx_.CheckConfiguration(ctx_.pkt_type_config_map,
			worker_config.output_file_paths_, enabled_paths);
		if (!config_ok)
		{
			complete_ = true;
			return;
		}

		// For each packet type that is enabled and has an output path specified,
		// create a file writer object that is owned by Ch10Context to maintain
		// state between regular and append mode calls to this worker's operator().
		// Pass a pointer to the file writer to the relevant parser for use in 
		// writing data to disk.
		ctx_.InitializeFileWriters(enabled_paths);
	}

	// Instantiate Ch10Packet object
	Ch10Packet packet(&worker_config.bb_, &ctx_, tmats_body_vec);

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

	// Update last_position_;
	worker_config.last_position_ = ctx_.absolute_position;

	// Close all file writers if append_mode is true or
	// this is the final worker which has no append mode.
	if (worker_config.append_mode_ || worker_config.final_worker_)
	{
		SPDLOG_DEBUG("({:02d}) Closing file writers", worker_config.worker_index_);
		ctx_.CloseFileWriters();
	}
	
	SPDLOG_INFO("({:02d}) End of worker's shift", worker_config.worker_index_);
	SPDLOG_DEBUG("({:02d}) End of shift, absolute position: {:d}", 
		worker_config.worker_index_, worker_config.last_position_);
	complete_ = true;
}

std::atomic<bool>& ParseWorker::completion_status()
{
	return complete_;
}

void ParseWorker::reset_completion_status()
{
	complete_ = false;
}

//uint16_t ParseWorker::get_binbuff_ind()
//{ return buffer_index_; }
//
//uint64_t& ParseWorker::get_last_position()
//{
//	return last_position_;
//}

void ParseWorker::append_chanid_remoteaddr_maps(std::map<uint32_t, std::set<uint16_t>>& out1,
	std::map<uint32_t, std::set<uint16_t>>&out2)
{
	IterableTools it;
	out1 = it.CombineCompoundMapsToSet(out1, ctx_.chanid_remoteaddr1_map);
	out2 = it.CombineCompoundMapsToSet(out2, ctx_.chanid_remoteaddr2_map);
}

void ParseWorker::append_chanid_comwmwords_map(std::map<uint32_t, std::set<uint32_t>>& out)
{
	IterableTools it;
	out = it.CombineCompoundMapsToSet(out, ctx_.chanid_commwords_map);
}

const std::map<uint16_t, uint64_t>& ParseWorker::GetChannelIDToMinTimeStampMap()
{
	return ctx_.chanid_minvideotimestamp_map;
}
