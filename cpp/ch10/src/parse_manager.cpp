// parse_manager.cpp

#include "parse_manager.h"

ParseManager::ParseManager() :
	worker_count_(0), worker_count(worker_count_),
	worker_chunk_size_bytes_(0), worker_chunk_size_bytes(worker_chunk_size_bytes_),
	workers_vec(workers_vec_), threads_vec(threads_vec_), 
	worker_config_vec(worker_config_vec_), ch10_input_stream_(),
	append_chunk_size_bytes_(100000000),
	it_()
{
	
}

bool ParseManager::Configure(ManagedPath input_ch10_file_path, ManagedPath output_dir,
	const ParserConfigParams& user_config)
{
	bool success = false;
	input_ch10_file_path.GetFileSize(success, ch10_file_size_);
	if (!success)
		return false;
	spdlog::get("pm_logger")->info("Ch10 file size: {:d} MiB", ch10_file_size_ / (1024 * 1024));

	// Convert ch10_packet_type configuration map from string --> string to 
	// Ch10PacketType --> bool
	std::map<Ch10PacketType, bool> packet_type_config_map;
	if (!ConvertCh10PacketTypeMap(user_config.ch10_packet_type_map_, packet_type_config_map))
		return false;

	// Hard-code packet type directory extensions now. Later, import from
	// config yaml.
	std::map<Ch10PacketType, std::string> append_str_map = {
		{Ch10PacketType::MILSTD1553_F1, "_1553.parquet"},
		{Ch10PacketType::VIDEO_DATA_F0, "_video.parquet"}
	};

	if (!CreateCh10PacketOutputDirs(output_dir, input_ch10_file_path,
		packet_type_config_map, append_str_map, output_dir_map_, true))
		return false;

	if (!AllocateResources(user_config, ch10_file_size_))
		return false;

	// Create output file names. A map of Ch10PacketType to
	// ManagedPath will be created for each worker.
	CreateCh10PacketWorkerFileNames(worker_count_, output_dir_map_, output_file_path_vec_,
		"parquet");

	// Record the packet type config map in metadata and logs.
	LogPacketTypeConfig(packet_type_config_map);

	// Open the input file stream
	ch10_input_stream_.open(input_ch10_file_path.string().c_str(), std::ios::binary);
	if (!(ch10_input_stream_.is_open()))
	{
		spdlog::get("pm_logger")->error("Error opening file: {:s}", 
			input_ch10_file_path.RawString());
		ch10_input_stream_.close();
		return false;
	}

	return true;
}

bool ParseManager::AllocateResources(const ParserConfigParams& user_config,
	const uint64_t& ch10_file_size)
{
	/*
	Note that ParserConfigParams has limits applied to the parameters so there
	is no need to check range, etc.
	*/

	// Multiply by 1e6 because this configuration parameters is in units
	// of MB.
	worker_chunk_size_bytes_ = user_config.parse_chunk_bytes_ * 1e6;

	// Calculate the number of workers necessary to parse the entire file
	// based on the chunk of binary that each worker will parse.
	worker_count_ = int(ceil(float(ch10_file_size) / float(worker_chunk_size_bytes_)));

	spdlog::get("pm_logger")->info("Using {:d} threads", user_config.parse_thread_count_);
	

	// If the user-specified max_chunk_read_count is less the calculated worker_count_,
	// decrease the worker_count_ to max_chunk_read_count.
	if (user_config.max_chunk_read_count_ < worker_count_)
		worker_count_ = user_config.max_chunk_read_count_;
	spdlog::get("pm_logger")->debug("AllocateResources: creating {:d} workers", worker_count_);

	// Allocate objects necessary to parse each chunk
	spdlog::get("pm_logger")->debug("AllocateResources: allocating memory for threads "
		"and WorkerConfig objects");
	threads_vec_.resize(worker_count_);
	worker_config_vec_.resize(worker_count_);

	// Allocate worker object for each unique_ptr<ParseWorker>
	spdlog::get("pm_logger")->debug("AllocateResources: creating ParseWorker objects");
	for (uint16_t worker_ind = 0; worker_ind < worker_count_; worker_ind++)
	{
		workers_vec_.push_back(std::make_unique<ParseWorker>());
	}

	return true;
}

bool ParseManager::Parse(const ParserConfigParams& user_config)
{
	// Convert ch10_packet_type configuration map from string --> string to 
	// Ch10PacketType --> bool
	std::map<Ch10PacketType, bool> packet_type_config_map;
	if (!ConvertCh10PacketTypeMap(user_config.ch10_packet_type_map_, packet_type_config_map))
		return false;

	// Start queue to activate all workers, limiting the quantity of 
	// concurrent threads to n_threads.
	bool append = false;
	std::vector<uint16_t> active_workers_vec;
	spdlog::get("pm_logger")->debug("Parse: begin parsing with workers");
	if (!WorkerQueue(append, ch10_input_stream_, workers_vec_, active_workers_vec,
		worker_config_vec_, worker_chunk_size_bytes_, 
		append_chunk_size_bytes_, ch10_file_size_, output_file_path_vec_, 
		packet_type_config_map, threads_vec_, tmats_body_vec_, user_config))
	{
		spdlog::get("pm_logger")->warn("Parse: Returning after first WorkerQueue");
		return false;
	}
	spdlog::get("pm_logger")->debug("Parse: end parsing with workers");

	// Wait for all active workers to finish.
	if (!WorkerRetireQueue(workers_vec_, active_workers_vec, worker_config_vec_,
		threads_vec_, user_config.worker_shift_wait_ms_))
	{
		return false;
	}

	// After all workers have finished and joined, each worker except the
	// last will parse the dangling packets that occur at the end of each 
	// worker's section of raw data and at the beginning of the following 
	// worker's section of data that occurs at the beginning of it's data
	// payload before it finds the time data packet to establish absolute
	// time. 

	// Delete current threads, then allocate new threads for each worker.
	// Quantity of threads is n_reads-1 because the last worker, which 
	// reaches the end of the file, does not leave dangling packets and thus
	// doesn't require another thread to be run again. 
	threads_vec_.clear();
	threads_vec_.resize(worker_count_ - 1);

	spdlog::get("pm_logger")->debug("Parse: begin parsing in append mode");
	append = true;
	if (!WorkerQueue(append, ch10_input_stream_, workers_vec_, active_workers_vec,
		worker_config_vec_, worker_chunk_size_bytes_, 
		append_chunk_size_bytes_, ch10_file_size_, output_file_path_vec_, 
		packet_type_config_map, threads_vec_, tmats_body_vec_, user_config))
	{
		spdlog::get("pm_logger")->warn("Parse: Returning after append mode WorkerQueue");
		return false;
	}
	spdlog::get("pm_logger")->debug("Parse: end parsing in append mode");

	// Wait for all active workers to finish.
	if (!WorkerRetireQueue(workers_vec_, active_workers_vec, worker_config_vec_,
		threads_vec_, user_config.worker_shift_wait_ms_))
	{
		return false;
	}

	return true;
}

bool ParseManager::RecordMetadata(ManagedPath input_ch10_file_path, 
	const ParserConfigParams& user_config)
{
	// Convert ch10_packet_type configuration map from string --> string to 
	// Ch10PacketType --> bool
	std::map<Ch10PacketType, bool> packet_type_config_map;
	if (!ConvertCh10PacketTypeMap(user_config.ch10_packet_type_map_, packet_type_config_map))
		return false;

	spdlog::get("pm_logger")->debug("RecordMetadata: begin record metadata");
	
	if (packet_type_config_map.count(Ch10PacketType::MILSTD1553_F1) == 1)
	{
		if (packet_type_config_map.at(Ch10PacketType::MILSTD1553_F1))
		{
			spdlog::get("pm_logger")->debug("RecordMetadata: recording {:s} metadata",
				ch10packettype_to_string_map.at(Ch10PacketType::MILSTD1553_F1));
			Metadata md;
			ManagedPath md_path = md.GetYamlMetadataPath(
				output_dir_map_[Ch10PacketType::MILSTD1553_F1],
				"_metadata");

			// Record Config options used
			md.RecordSimpleMap(user_config.ch10_packet_type_map_, "ch10_packet_type");
			md.RecordSingleKeyValuePair("parse_chunk_bytes", user_config.parse_chunk_bytes_);
			md.RecordSingleKeyValuePair("parse_thread_count", user_config.parse_thread_count_);
			md.RecordSingleKeyValuePair("max_chunk_read_count", user_config.max_chunk_read_count_);
			md.RecordSingleKeyValuePair("worker_offset_wait_ms", user_config.worker_offset_wait_ms_);
			md.RecordSingleKeyValuePair("worker_shift_wait_ms", user_config.worker_shift_wait_ms_);

			// Record the input ch10 path.
			md.RecordSingleKeyValuePair("ch10_input_file_path", input_ch10_file_path.RawString());

			// Obtain the tx and rx combined channel ID to LRU address map and
			// record it to the Yaml writer.
			std::map<uint32_t, std::set<uint16_t>> output_chanid_remoteaddr_map;
			collect_chanid_to_lruaddrs_metadata(output_chanid_remoteaddr_map,
				workers_vec_, worker_count_);
			md.RecordCompoundMapToSet(output_chanid_remoteaddr_map, "chanid_to_lru_addrs");

			// Obtain the channel ID to command words set map.
			std::map<uint32_t, std::vector<std::vector<uint32_t>>> output_chanid_commwords_map;
			collect_chanid_to_commwords_metadata(output_chanid_commwords_map,
				workers_vec_, worker_count_);
			md.RecordCompoundMapToVectorOfVector(output_chanid_commwords_map, "chanid_to_comm_words");

			std::map<std::string, std::string> TMATsChannelIDToSourceMap;
			std::map<std::string, std::string> TMATsChannelIDToTypeMap;
			ProcessTMATS();

			// Record the TMATS channel ID to source map.
			md.RecordSimpleMap(TMATsChannelIDToSourceMap, "tmats_chanid_to_source");

			// Record the TMATS channel ID to type map.
			md.RecordSimpleMap(TMATsChannelIDToTypeMap, "tmats_chanid_to_type");

			// Write the complete Yaml record to the metadata file.
			std::ofstream stream_1553_metadata(md_path.string(),
				std::ofstream::out | std::ofstream::trunc);
			stream_1553_metadata << md.GetMetadataString();
			stream_1553_metadata.close();
		}
	}

	if (packet_type_config_map.count(Ch10PacketType::VIDEO_DATA_F0) == 1)
	{
		if (packet_type_config_map.at(Ch10PacketType::VIDEO_DATA_F0))
		{
			spdlog::get("pm_logger")->debug("RecordMetadata: recording {:s} metadata",
				ch10packettype_to_string_map.at(Ch10PacketType::VIDEO_DATA_F0));
			Metadata vmd;
			ManagedPath video_md_path = vmd.GetYamlMetadataPath(
				output_dir_map_[Ch10PacketType::VIDEO_DATA_F0],
				"_metadata");

			// Get the channel ID to minimum time stamp map.
			std::map<uint16_t, uint64_t> min_timestamp_map;
			CollectVideoMetadata(min_timestamp_map, workers_vec_, worker_count_);

			// Record the map in the Yaml writer and write the 
			// total yaml text to file.
			vmd.RecordSimpleMap(min_timestamp_map, "chanid_to_first_timestamp");
			std::ofstream stream_video_metadata(video_md_path.string(),
				std::ofstream::out | std::ofstream::trunc);
			stream_video_metadata << vmd.GetMetadataString();
			stream_video_metadata.close();
		}
	}
	spdlog::get("pm_logger")->debug("RecordMetadata: complete record metadata");
	return true;
}

bool ParseManager::ConfigureWorker(WorkerConfig& worker_config, const uint16_t& worker_index,
	const uint16_t& worker_count, const uint64_t& read_pos, const uint64_t& read_size,
	const std::map<Ch10PacketType, ManagedPath>& output_file_path_map,
	const std::map<Ch10PacketType, bool>& packet_type_config_map)
{
	if (worker_index > worker_count - 1)
	{
		spdlog::get("pm_logger")->warn("ConfigureWorker: worker_index ({:d}) > worker_count ({:d}) - 1",
			worker_index, worker_count);
		return false;
	}

	worker_config.final_worker_ = false;
	if (worker_index == worker_count - 1)
	{
		worker_config.final_worker_ = true;
	}

	worker_config.worker_index_ = worker_index;
	worker_config.start_position_ = read_pos;
	worker_config.append_mode_ = false;
	worker_config.output_file_paths_ = output_file_path_map;
	worker_config.ch10_packet_type_map_ = packet_type_config_map;

	spdlog::get("pm_logger")->debug("ConfigureWorker {:d}: start = {:d}, read size = {:d}",
		worker_index, read_pos, read_size);

	return true;
}

void ParseManager::ConfigureAppendWorker(WorkerConfig& worker_config, const uint16_t& worker_index,
	const uint64_t& append_read_size)
{
	uint64_t last_pos = worker_config.last_position_;

	spdlog::get("pm_logger")->debug("ConfigureAppendWorker {:d}: start = {:d}, read size = {:d}",
		worker_index, last_pos, append_read_size);

	worker_config.start_position_ = last_pos;
	worker_config.append_mode_ = true;
}



bool ParseManager::WorkerQueue(bool append_mode, std::ifstream& ch10_input_stream,
	std::vector<std::unique_ptr<ParseWorker>>& worker_vec, 
	std::vector<uint16_t>& active_workers_vec,
	std::vector<WorkerConfig>& worker_config_vec,
	const uint64_t& read_size, const uint64_t& append_read_size, 
	const uint64_t& total_size,
	const std::vector<std::map<Ch10PacketType, ManagedPath>>& output_file_path_vec,
	const std::map<Ch10PacketType, bool>& packet_type_config_map,
	std::vector<std::thread>& threads_vec,
	std::vector<std::string>& tmats_vec,
	const ParserConfigParams& user_config)
{
	// Load sleep duration from user config
	std::chrono::milliseconds worker_offset_wait_ms(user_config.worker_offset_wait_ms_);
	std::chrono::milliseconds worker_shift_wait_ms(user_config.worker_shift_wait_ms_);

	uint16_t worker_count = worker_vec.size();

	// Initially load all threads by keeping track of the
	// the number activated. Afterwards only activate new 
	// threads when an active thread finishes.
	uint16_t active_thread_count = 0;
	uint16_t current_active_worker = 0;
	bool thread_started = false;
	bool all_threads_active = false;
	bool eof_reached = false;
	std::streamsize actual_read_size = 0;
	uint16_t bb_ind = 0;
	int concurrent_thread_count = 0;
	uint64_t total_read_pos = 0;
	spdlog::get("pm_logger")->debug("WorkerQueue: Starting worker threads");

	// Create and wait for workers until the entire input file 
	// is parsed.

	// Loop over each block of data that is to be read and parsed. There is a 1:1
	// relationship between a worker and a block of data--each worker consumes a 
	// single data block. In the append_mode = true
	// case, the very last worker doesn't have any dangling packets because presumably
	// it parsed data until the ch10 file ended. Only [worker 1, worker last) need to
	// append data to their respective files. 
	uint16_t n_read_limit = worker_count;
	if (append_mode)
	{
		// If there is only 1 worker, then there is no need to 
		// append data. Also, there is no reason to append data
		// to the last worker (i.e., the worker that read the last 
		// portion of the Ch10 file) so only append data for all
		// workers except the last worker. 
		if (n_read_limit > 1)
			n_read_limit--;
		else
			return true;
	}
	for (uint16_t worker_ind = 0; worker_ind < n_read_limit; worker_ind++)
	{
		thread_started = false;
		// Stay in while loop until another worker is started. 
		while (!thread_started)
		{
			if (!all_threads_active)
			{
				spdlog::get("pm_logger")->debug("WorkerQueue: All threads NOT ACTIVE ({:d} active)",
					active_thread_count);

				// Immediately activate a new worker.
				if (append_mode)
				{
					ConfigureAppendWorker(worker_config_vec[worker_ind], worker_ind, 
						append_read_size);

					actual_read_size = worker_config_vec[worker_ind].bb_.Initialize(ch10_input_stream,
						total_size, worker_config_vec[worker_ind].start_position_, append_read_size);
				}
				else
				{
					if (!ConfigureWorker(worker_config_vec[worker_ind], worker_ind, worker_count,
						total_read_pos, read_size, output_file_path_vec[worker_ind],
						packet_type_config_map))
					{
						spdlog::get("pm_logger")->warn("WorkerQueue: ConfigureWorker failed during "
							"initial thread loading");
						return false;
					}

					actual_read_size = worker_config_vec[worker_ind].bb_.Initialize(ch10_input_stream,
						total_size, total_read_pos, read_size);
				}

				threads_vec[worker_ind] = std::thread(std::ref(*worker_vec[worker_ind]),
					std::ref(worker_config_vec[worker_ind]), std::ref(tmats_vec));

				// Check if the correct number of bytes were read.
				if (append_mode)
				{
					if (actual_read_size != append_read_size)
					{
						spdlog::get("pm_logger")->warn(
							"WorkerQueue: worker {:d} Error: {:d} bytes read, {:d} bytes indicated",
							worker_ind, actual_read_size, append_read_size);
						return false;
					}
				}
				else if (actual_read_size != read_size)
				{
					eof_reached = true;
					spdlog::get("pm_logger")->debug("WorkerQueue: EOF reached: {:d}/{:d} read",
						actual_read_size, read_size);
				}

				// Put worker in active workers list.
				active_workers_vec.push_back(worker_ind);

				active_thread_count += 1;
				if (active_thread_count == user_config.parse_thread_count_)
					all_threads_active = true;

				thread_started = true;

				// Stagger start time of initial worker threads. Because workers parse large chunks
				// of data and there are potentially 1e5 or 1e6 messages, the law of averages applies
				// and workers tend to take nearly identical amount of time to complete. This means
				// that the OS tries to write any remaining unwritten data to disk for each worker
				// then start a series of new workers which requires reading large amount of data 
				// from the disk, which results in an IO bottleneck that occurs every time the current
				// shift of workers finishes. Stagger the initial start time of workers to avoid this
				// bottleneck.
				if (!append_mode && active_thread_count != user_config.parse_thread_count_)
					std::this_thread::sleep_for(worker_offset_wait_ms);
			}
			else
			{
				spdlog::get("pm_logger")->debug("All threads ACTIVE ({:d} active)", active_thread_count);
				// Check active workers to see if they are ready to be joined.
				for (uint16_t active_worker_ind = 0; active_worker_ind < active_workers_vec.size(); 
					active_worker_ind++)
				{
					current_active_worker = active_workers_vec[active_worker_ind];
					// Join workers that are complete
					if (workers_vec[current_active_worker]->CompletionStatus() == true)
					{
						spdlog::get("pm_logger")->debug("WorkerQueue: worker {:d} "
							"INACTIVE/COMPLETE -- joining now", current_active_worker);

						threads_vec[current_active_worker].join();
						active_workers_vec.erase(active_workers_vec.begin() + active_worker_ind);

						// Clear the buffer
						worker_config_vec[current_active_worker].bb_.Clear();

						// Immediately start a new worker.
						if (append_mode)
						{
							ConfigureAppendWorker(worker_config_vec[worker_ind], worker_ind,
								append_read_size);

							actual_read_size = worker_config_vec[worker_ind].bb_.Initialize(ch10_input_stream,
								total_size, worker_config_vec[worker_ind].start_position_, append_read_size);
						}
						else
						{
							if (!ConfigureWorker(worker_config_vec[worker_ind], worker_ind, worker_count,
								total_read_pos, read_size, output_file_path_vec[worker_ind],
								packet_type_config_map))
							{
								spdlog::get("pm_logger")->warn("WorkerQueue: ConfigureWorker failed during "
									"opportunistic thread loading");
								return false;
							}

							actual_read_size = worker_config_vec[worker_ind].bb_.Initialize(ch10_input_stream,
								total_size, total_read_pos, read_size);
						}

						threads_vec[worker_ind] = std::thread(std::ref(*worker_vec[worker_ind]),
							std::ref(worker_config_vec[worker_ind]), std::ref(tmats_vec));

						// Check if the correct number of bytes were read.
						if (append_mode)
						{
							if (actual_read_size != append_read_size)
							{
								spdlog::get("pm_logger")->warn(
									"WorkerQueue: worker {:d} Error: {:d} bytes read, {:d} bytes indicated",
									worker_ind, actual_read_size, append_read_size);
								return false;
							}
						}
						else if (actual_read_size != read_size)
						{
							eof_reached = true;
							spdlog::get("pm_logger")->debug("WorkerQueue: EOF reached: {:d}/{:d} read",
								actual_read_size, read_size);
						}

						// Place new worker among active workers.
						active_workers_vec.push_back(worker_ind);

						thread_started = true;
						break;
					}
					else
					{
						spdlog::get("pm_logger")->debug("WorkerQueue: worker {:d} STILL ACTIVE",
							current_active_worker);
					}

				}
			}

			// Wait before checking for available workers.
			spdlog::get("pm_logger")->trace("WorkerQueue: waiting for workers");
			std::this_thread::sleep_for(worker_shift_wait_ms);

		} // end while 

		// Increase the total read position.
		total_read_pos += actual_read_size;

		if (eof_reached)
			break;

	} // end for loop over all worker indices.
	return true;
}

bool ParseManager::WorkerRetireQueue(std::vector<std::unique_ptr<ParseWorker>>& worker_vec,
	std::vector<uint16_t>& active_workers_vec,
	std::vector<WorkerConfig>& worker_config_vec,
	std::vector<std::thread>& threads_vec,
	int worker_shift_wait)
{
	uint16_t worker_count = worker_vec.size();
	std::chrono::milliseconds worker_shift_wait_ms(worker_shift_wait);
	spdlog::get("pm_logger")->debug("WorkerRetireQueue: Joining all remaining workers");
	uint16_t current_active_worker = 0;

	while (active_workers_vec.size() > 0)
	{
		for (uint16_t active_worker_ind = 0; active_worker_ind < active_workers_vec.size(); 
			active_worker_ind++)
		{
			// Join workers that are complete
			current_active_worker = active_workers_vec[active_worker_ind];
			if (workers_vec[current_active_worker]->CompletionStatus() == true)
			{
				spdlog::get("pm_logger")->debug("WorkerRetireQueue: worker {:d} "
					"INACTIVE/COMPLETE -- joining now", current_active_worker);

				threads_vec[current_active_worker].join();

				// Clear the binary buffer to free memory
				worker_config_vec[current_active_worker].bb_.Clear();

				spdlog::get("pm_logger")->debug("WorkerRetireQueue: worker {:d} joined", 
					current_active_worker);
				if (worker_count == 1)
					active_workers_vec.resize(0);
				else
					active_workers_vec.erase(active_workers_vec.begin() + active_worker_ind);
			}
			else
			{
				spdlog::get("pm_logger")->debug("WorkerRetireQueue: worker {:d} STILL ACTIVE", 
					current_active_worker);
			}
		}

		// Wait before checking for available workers.
		spdlog::get("pm_logger")->debug("WorkerRetireQueue: waiting for workers to complete");
		std::this_thread::sleep_for(worker_shift_wait_ms);
	}

	spdlog::get("pm_logger")->debug("WorkerRetireQueue: all workers joined");
	return true;
}

ParseManager::~ParseManager()
{
	ch10_input_stream_.close();
}

void ParseManager::ProcessTMATS(const std::vector<std::string>& tmats_vec,
	const std::map<Ch10PacketType, ManagedPath>& pkt_type_output_dir_map,
	std::map<std::string, std::string>& TMATsChannelIDToSourceMap,
	std::map<std::string, std::string>& TMATsChannelIDToTypeMap)
{
	// if tmats doesn't exist return
	if (tmats_vec.size() == 0)
	{
		spdlog::get("pm_logger")->warn("ProcessTMATS: no TMATS Present");
		return;
	}

	std::string full_TMATS_string;
	for (int i = 0; i < tmats_vec.size(); i++)
	{
		full_TMATS_string += tmats_vec[i];
	}

	ManagedPath tmats_path;
	tmats_path = pkt_type_output_dir_map[Ch10PacketType::MILSTD1553_F1] /
		"_TMATS.txt";
	std::ofstream tmats;
	tmats.open(tmats_path.string(), std::ios::trunc | std::ios::binary);
	if (tmats.good())
	{
		spdlog::get("pm_logger")->info("ProcessTMATS: writing TMATS to {:s}", tmats_path.RawString());
		tmats << full_TMATS_string;
	}

	tmats.close();

	// Gather TMATs attributes of interest
	// for metadata
	TMATSParser tmats_parser = TMATSParser(full_TMATS_string);
	TMATsChannelIDToSourceMap = tmats_parser.MapAttrs("R-x\\TK1-n", "R-x\\DSI-n");
	TMATsChannelIDToTypeMap = tmats_parser.MapAttrs("R-x\\TK1-n", "R-x\\CDT-n");
}

bool ParseManager::ConvertCh10PacketTypeMap(const std::map<std::string, std::string>& input_map,
	std::map<Ch10PacketType, bool>& output_map)
{
	// The ch10 can't be parsed if there is no packet type configuration.
	// If the input_map is empty, return false.
	if (input_map.size() == 0)
		return false;

	// Define string to Ch10PacketType map
	std::map<std::string, Ch10PacketType> conversion_map = {
		{"MILSTD1553_FORMAT1", Ch10PacketType::MILSTD1553_F1},
		{"VIDEO_FORMAT0", Ch10PacketType::VIDEO_DATA_F0}
	};

	ParseText pt;
	std::string bool_string; 
	for (std::map<std::string, std::string>::const_iterator it = input_map.cbegin();
		it != input_map.cend(); ++it)
	{
		// If the name of the packet type from the conversion map is not present,
		// clear the output map and return false.
		if (conversion_map.count(it->first) == 0)
		{
			output_map.clear();
			spdlog::get("pm_logger")->warn("ch10_packet_type configuration key {:s} not "
				"in conversion_map", it->first);
			return false;
		}

		// Check for valid boolean string
		bool_string = pt.ToLower(it->second);
		if (bool_string == "true")
			output_map[conversion_map.at(it->first)] = true;
		else if (bool_string == "false")
			output_map[conversion_map.at(it->first)] = false;
		else
		{
			output_map.clear();
			spdlog::get("pm_logger")->warn("ch10_packet_type configuration boolean "
				"string {:s} not valid, must spell \"true\" or \"false\", upper or "
				"lower chars", it->second);
			return false;
		}
	}
	return true;
}

void ParseManager::LogPacketTypeConfig(const std::map<Ch10PacketType, bool>& pkt_type_config_map)
{
	// Convert the Ch10PacketType to bool --> string to bool
	IterableTools iter_tools;
	std::map<std::string, bool> str_packet_type_map;
	for (std::map<Ch10PacketType, bool>::const_iterator it = pkt_type_config_map.cbegin();
		it != pkt_type_config_map.cend(); ++it)
	{
		str_packet_type_map[ch10packettype_to_string_map.at(it->first)] = it->second;
	}

	// Get string representation of key-value pairs
	std::string stringrepr = iter_tools.GetPrintableMapElements_KeyToBool(
		str_packet_type_map);

	// Log the map information.
	spdlog::get("pm_logger")->info("Ch10 packet configuration:");
	ParseText pt;
	std::vector<std::string> splitstr = pt.Split(stringrepr, '\n');
	for (std::vector<std::string>::const_iterator it = splitstr.cbegin();
		it != splitstr.cend(); ++it)
	{
		spdlog::get("pm_logger")->info(*it);
	}
}

bool ParseManager::CreateCh10PacketOutputDirs(const ManagedPath& output_dir,
	const ManagedPath& base_file_name,
	const std::map<Ch10PacketType, bool>& packet_enabled_map,
	const std::map<Ch10PacketType, std::string>& append_str_map,
	std::map<Ch10PacketType, ManagedPath>& pkt_type_output_dir_map, bool create_dir)
{
	// Check for present of append_str_map entry for each packet_enabled_map
	// entry which maps to true.
	std::map<Ch10PacketType, bool>::const_iterator it;
	bool result = false;
	ManagedPath pkt_type_output_dir;
	for (it = packet_enabled_map.cbegin(); it != packet_enabled_map.cend(); ++it)
	{
		if (it->second)
		{
			if (append_str_map.count(it->first) == 0)
			{
				spdlog::get("pm_logger")->info("CreateCh10PacketOutputDirs: No append "
					"string map entry for {:s}", ch10packettype_to_string_map.at(it->first));
				return false;
			}

			// Fill pkt_type_output_dir_map for each packet type in packet_enabled_map.
			result = ManagedPath::CreateDirectoryFromComponents(output_dir, base_file_name,
				append_str_map.at(it->first), pkt_type_output_dir, create_dir);
			if (!result)
			{
				pkt_type_output_dir_map.clear();
				return false;
			}
			spdlog::get("pm_logger")->info("Create {:s} output dir: {:s}",
				ch10packettype_to_string_map.at(it->first), pkt_type_output_dir.RawString());
			pkt_type_output_dir_map[it->first] = pkt_type_output_dir;
		}
	}

	return true;
}

void ParseManager::CreateCh10PacketWorkerFileNames(const uint16_t& total_worker_count,
	const std::map<Ch10PacketType, ManagedPath>& pkt_type_output_dir_map,
	std::vector< std::map<Ch10PacketType, ManagedPath>>& output_vec_mapped_paths,
	std::string file_extension)
{
	std::string replacement_ext = "";
	std::map<Ch10PacketType, ManagedPath>::const_iterator it;

	for (uint16_t worker_index = 0; worker_index < total_worker_count; worker_index++)
	{
		// Create the replacement extension for the current index.
		std::stringstream ss;
		ss << "__" << std::setfill('0') << std::setw(3) << worker_index;
		if (file_extension != "")
		{
			ss << "." << file_extension;
		}
		replacement_ext = ss.str();

		// Create a temporary map to hold all of the output file paths for the 
		// current index.
		std::map<Ch10PacketType, ManagedPath> temp_output_file_map;

		// Add an output file path for each Ch10PacketType and output dir
		// in pkt_type_output_dir_map.
		for (it = pkt_type_output_dir_map.cbegin(); it != pkt_type_output_dir_map.cend(); ++it)
		{
			temp_output_file_map[it->first] = it->second.CreatePathObject(
				it->second, replacement_ext);
		}

		// Add the temp map to the vector maps if the temp map has items.
		if(temp_output_file_map.size() > 0)
			output_vec_mapped_paths.push_back(temp_output_file_map);
	}
}

void ParseManager::CollectVideoMetadata(
	std::map<uint16_t, uint64_t>& channel_id_to_min_timestamp_map,
	std::vector<std::unique_ptr<ParseWorker>>& worker_vec,
	const uint16_t& worker_count)
{
	// Gather the maps from each worker and combine them into one, 
	//keeping only the lowest time stamps for each channel ID.
	for (uint16_t worker_ind = 0; worker_ind < worker_count; worker_ind++)
	{
		std::map<uint16_t, uint64_t> temp_map =
			worker_vec[worker_ind]->ch10_context_.chanid_minvideotimestamp_map;
		for (std::map<uint16_t, uint64_t>::const_iterator it = temp_map.begin();
			it != temp_map.end(); ++it)
		{
			if (channel_id_to_min_timestamp_map.count(it->first) == 0)
				channel_id_to_min_timestamp_map[it->first] = it->second;
			else if (it->second < channel_id_to_min_timestamp_map[it->first])
				channel_id_to_min_timestamp_map[it->first] = it->second;
		}
	}
}

void ParseManager::collect_chanid_to_lruaddrs_metadata(
	std::map<uint32_t, std::set<uint16_t>>& output_chanid_remoteaddr_map,
	std::vector<std::unique_ptr<ParseWorker>>& worker_vec,
	const uint16_t& worker_count)
{
	// Collect and combine the channel ID to LRU address maps
	// assembled by each worker.
	std::map<uint32_t, std::set<uint16_t>> chanid_remoteaddr_map1;
	std::map<uint32_t, std::set<uint16_t>> chanid_remoteaddr_map2;
	for (uint16_t worker_ind = 0; worker_ind < worker_count; worker_ind++)
	{
		//workers[read_ind].append_chanid_remoteaddr_maps(chanid_remoteaddr_map1, chanid_remoteaddr_map2);
		chanid_remoteaddr_map1 = it_.CombineCompoundMapsToSet(
			chanid_remoteaddr_map1, worker_vec[worker_ind]->ch10_context_.chanid_remoteaddr1_map);
		chanid_remoteaddr_map2 = it_.CombineCompoundMapsToSet(
			chanid_remoteaddr_map2, worker_vec[worker_ind]->ch10_context_.chanid_remoteaddr2_map);
	}

	// Combine the tx and rx maps into a single map.
	output_chanid_remoteaddr_map = it_.CombineCompoundMapsToSet(
		chanid_remoteaddr_map1, chanid_remoteaddr_map2);
}

void ParseManager::collect_chanid_to_commwords_metadata(
	std::map<uint32_t, std::vector<std::vector<uint32_t>>>& output_chanid_commwords_map,
	std::vector<std::unique_ptr<ParseWorker>>& worker_vec,
	const uint16_t& worker_count)
{
	// Collect maps into one.
	std::map<uint32_t, std::set<uint32_t>> chanid_commwords_map;
	for (uint16_t worker_ind = 0; worker_ind < worker_count; worker_ind++)
	{
		//workers[read_ind].append_chanid_comwmwords_map(chanid_commwords_map);
		chanid_commwords_map = it_.CombineCompoundMapsToSet(chanid_commwords_map,
			worker_vec[worker_ind]->ch10_context_.chanid_commwords_map);
	}

	// Break compound command words each into a set of two command words,
	// a transmit and receive value.
	uint32_t mask_val = (1 << 16) - 1;
	for (std::map<uint32_t, std::set<uint32_t>>::const_iterator it = chanid_commwords_map.cbegin();
		it != chanid_commwords_map.cend(); ++it)
	{
		std::vector<std::vector<uint32_t>> temp_vec_of_vec;
		for (std::set<uint32_t>::const_iterator it2 = it->second.cbegin();
			it2 != it->second.cend(); ++it2)
		{
			// Vector needed here to retain order.
			std::vector<uint32_t> pair_vec = { *it2 >> 16, *it2 & mask_val };
			temp_vec_of_vec.push_back(pair_vec);
		}
		output_chanid_commwords_map[it->first] = temp_vec_of_vec;
	}
}