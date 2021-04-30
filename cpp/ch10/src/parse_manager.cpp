// parse_manager.cpp

#include "parse_manager.h"

ParseManager::ParseManager(ManagedPath fname, ManagedPath output_path, const ParserConfigParams * const config) :
	config_(config),
	input_path(fname), 
	output_path(output_path), 
	read_size(10000), 
	append_read_size(10000000), 
	total_size(0), 
	total_read_pos(0),
	n_threads(1), 
	error_set(false),
	workers_allocated(false),
	worker_wait(config->worker_shift_wait_ms_),
	worker_start_offset(config->worker_offset_wait_ms_), 
	ifile(),
	binary_buffers(nullptr), 
	check_word_count(true),
	n_reads(0), 
	threads(nullptr), 
	workers(nullptr), 
	milstd1553_msg_selection(false)
{
	bool success = false;
	input_path.GetFileSize(success, total_size);
	if (success)
	{
		spdlog::get("pm_logger")->info("File size: {:d} MB", total_size / (1024 * 1024));
		create_output_dirs();
	}
	else
		error_set = true;

#ifdef PARSER_REWRITE
	// Convert ch10_packet_type configuration map.
	success = ConvertCh10PacketTypeMap(config_->ch10_packet_type_map_, packet_type_config_map_);
	if (!success)
		error_set = true;
	LogPacketTypeConfig(packet_type_config_map_);
#endif
}

void ParseManager::create_output_dirs()
{
	ManagedPath parquet_1553_path = output_path.CreatePathObject(input_path, "_1553.parquet");
	spdlog::get("pm_logger")->info("Parquet 1553 output path: {:s}", 
		parquet_1553_path.RawString());
	if (!parquet_1553_path.create_directory())
		error_set = true;
	output_dir_map_[Ch10DataType::MILSTD1553_DATA_F1] = parquet_1553_path;

#ifdef VIDEO_DATA

	ManagedPath parquet_vid_path = output_path.CreatePathObject(input_path, "_video.parquet");
	spdlog::get("pm_logger")->info("Parquet video output path: {:s}", 
		parquet_vid_path.RawString());
	if (!parquet_vid_path.create_directory())
		error_set = true;
	output_dir_map_[Ch10DataType::VIDEO_DATA_F0] = parquet_vid_path;

#endif

#ifdef ETHERNET_DATA

	ManagedPath parquet_eth_path = output_path.CreatePathObject(input_path, "_ethernet.parquet");
	spdlog::get("pm_logger")->info("Parquet ethernet output path: {:s}", 
		parquet_eth_path.RawString());
	if (!parquet_eth_path.create_directory())
		error_set = true;
	output_dir_map_[Ch10DataType::ETHERNET_DATA_F0] = parquet_eth_path;

#endif
}

void ParseManager::create_output_file_paths()
{
	char replacement_ext_format[] = "__%03u.parquet";
	char replacement_ext_buffer[20];
	std::string replacement_ext = "";
	ManagedPath temp_output_dir;

	for (uint16_t read_ind = 0; read_ind < n_reads; read_ind++)
	{
		// Create the replacement extension for the current index.
		sprintf(replacement_ext_buffer, replacement_ext_format, read_ind);
		replacement_ext = std::string(replacement_ext_buffer);

		// Create a temporary map to hold all of the output file paths for the 
		// current index.
		std::map<Ch10DataType, ManagedPath> temp_output_file_map;

		// Get a copy of the output dir path for simplification.
		temp_output_dir = output_dir_map_[Ch10DataType::MILSTD1553_DATA_F1];

		// Create the output file path.
		temp_output_file_map[Ch10DataType::MILSTD1553_DATA_F1] = temp_output_dir.CreatePathObject(
			temp_output_dir, replacement_ext);

		/*printf("Create output file path: %s\n",
			temp_output_file_map[Ch10DataType::MILSTD1553_DATA_F1].RawString().c_str());*/

#ifdef VIDEO_DATA

		temp_output_dir = output_dir_map_[Ch10DataType::VIDEO_DATA_F0];
		temp_output_file_map[Ch10DataType::VIDEO_DATA_F0] = temp_output_dir.CreatePathObject(
			temp_output_dir, replacement_ext);

#endif

#ifdef ETHERNET_DATA

		temp_output_dir = output_dir_map_[Ch10DataType::ETHERNET_DATA_F0];
		temp_output_file_map[Ch10DataType::ETHERNET_DATA_F0] = temp_output_dir.CreatePathObject(
			temp_output_dir, replacement_ext);

#endif

		// Add the temp map to the vector maps.
		output_file_path_vec_.push_back(temp_output_file_map);
	}
}

bool ParseManager::error_state()
{
	return error_set;
}

void ParseManager::start_workers()
{
	ifile.open(input_path.string().c_str(), std::ios::binary);
	if (!(ifile.is_open()))
	{
		spdlog::get("pm_logger")->error("Error opening file: {:s}", input_path.RawString());
		error_set = true;
		return;
	}

	read_size = config_->parse_chunk_bytes_ * 1e6;
	n_threads = config_->parse_thread_count_;
	uint16_t max_workers = config_->max_chunk_read_count_;
	
	check_word_count = true;
	total_read_pos = 0;

	// Calculate the number of reads necessary to parse the entire file.
	n_reads = int(ceil(float(total_size) / float(read_size)));
	if (max_workers < n_reads)
		n_reads = max_workers;
	workers = new ParseWorker[n_reads];
	threads = new std::thread[n_reads];
	binary_buffers = new BinBuff[n_reads];
	workers_allocated = true;

	create_output_file_paths();

	spdlog::get("pm_logger")->info("Input file: {:s}", input_path.RawString());
	spdlog::get("pm_logger")->info("Using {:d} threads", n_threads);
	spdlog::get("pm_logger")->debug("Created {:d} workers", n_reads);
	spdlog::get("pm_logger")->debug("Created {:d} binary buffers", n_reads);

	// Start queue to activate all workers, limiting the quantity of 
	// concurrent threads to n_threads.
	bool append = false;
	worker_queue(append);
	spdlog::get("pm_logger")->debug("after worker_queue");

	// Wait for all active workers to finish.
	worker_retire_queue();

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
	delete[] threads;
	threads = new std::thread[n_reads];

	spdlog::get("pm_logger")->info("Parsing dangling packets");

	append = true;
	worker_queue(append);

	// Wait for all active workers to finish.
	worker_retire_queue();

#ifdef COLLECT_STATS
	collect_stats();
#endif
	
#ifdef PARQUET
	// Create metadata object and create output path name for metadata
	// to be recorded in 1553 output directory.
	Metadata md;
	ManagedPath md_path = md.GetYamlMetadataPath(
		output_dir_map_[Ch10DataType::MILSTD1553_DATA_F1],
		"_metadata");

	// Record Config options used
#ifdef PARSER_REWRITE
	md.RecordSimpleMap(config_->ch10_packet_type_map_, "ch10_packet_type");
#endif
	md.RecordSingleKeyValuePair("parse_chunk_bytes", config_->parse_chunk_bytes_);
	md.RecordSingleKeyValuePair("parse_thread_count", config_->parse_thread_count_);
	md.RecordSingleKeyValuePair("max_chunk_read_count", config_->max_chunk_read_count_);
	md.RecordSingleKeyValuePair("worker_offset_wait_ms", config_->worker_offset_wait_ms_);
	md.RecordSingleKeyValuePair("worker_shift_wait_ms", config_->worker_shift_wait_ms_);

	// Record the input ch10 path.
	md.RecordSingleKeyValuePair("ch10_input_file_path", input_path.RawString());

	// Obtain the tx and rx combined channel ID to LRU address map and
	// record it to the Yaml writer.
	std::map<uint32_t, std::set<uint16_t>> output_chanid_remoteaddr_map;
	collect_chanid_to_lruaddrs_metadata(output_chanid_remoteaddr_map);
	md.RecordCompoundMapToSet(output_chanid_remoteaddr_map, "chanid_to_lru_addrs");

	// Obtain the channel ID to command words set map.
	std::map<uint32_t, std::vector<std::vector<uint32_t>>> output_chanid_commwords_map;
	collect_chanid_to_commwords_metadata(output_chanid_commwords_map);
	md.RecordCompoundMapToVectorOfVector(output_chanid_commwords_map, "chanid_to_comm_words");

#if defined(LIBIRIG106) || defined(PARSER_REWRITE)
	ProcessTMATS();

	// Record the TMATS channel ID to source map.
	md.RecordSimpleMap(TMATsChannelIDToSourceMap_, "tmats_chanid_to_source");

	// Record the TMATS channel ID to type map.
	md.RecordSimpleMap(TMATsChannelIDToTypeMap_, "tmats_chanid_to_type");
#endif

	// Write the complete Yaml record to the metadata file.
	std::ofstream stream_1553_metadata(md_path.string(), 
		std::ofstream::out | std::ofstream::trunc);
	stream_1553_metadata << md.GetMetadataString();
	stream_1553_metadata.close();

#ifdef VIDEO_DATA
	// Create metadata object for video metadata.
	Metadata vmd;
	md_path = vmd.GetYamlMetadataPath(
		output_dir_map_[Ch10DataType::VIDEO_DATA_F0],
		"_metadata");

	// Get the channel ID to minimum time stamp map.
	std::map<uint16_t, uint64_t> min_timestamp_map;
	CollectVideoMetadata(min_timestamp_map);

	// Record the map in the Yaml writer and write the 
	// total yaml text to file.
	vmd.RecordSimpleMap(min_timestamp_map, "chanid_to_first_timestamp");
	std::ofstream stream_video_metadata(md_path.string(),
		std::ofstream::out | std::ofstream::trunc);
	stream_video_metadata << vmd.GetMetadataString();
	stream_video_metadata.close();
#endif
#endif
}

#ifdef VIDEO_DATA
void ParseManager::CollectVideoMetadata(
	std::map<uint16_t, uint64_t>& channel_id_to_min_timestamp_map)
{
	// Gather the maps from each worker and combine them into one, 
	//keeping only the lowest time stamps for each channel ID.
	for (int i = 0; i < n_reads; i++)
	{
		std::map<uint16_t, uint64_t> temp_map = workers[i].GetChannelIDToMinTimeStampMap();
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
#endif

void ParseManager::collect_chanid_to_lruaddrs_metadata(
	std::map<uint32_t, std::set<uint16_t>>& output_chanid_remoteaddr_map)
{
	// Collect and combine the channel ID to LRU address maps
	// assembled by each worker.
	std::map<uint32_t, std::set<uint16_t>> chanid_remoteaddr_map1;
	std::map<uint32_t, std::set<uint16_t>> chanid_remoteaddr_map2;
	for (uint16_t read_ind = 0; read_ind < n_reads; read_ind++)
	{
		workers[read_ind].append_chanid_remoteaddr_maps(chanid_remoteaddr_map1, chanid_remoteaddr_map2);
	}

	// Combine the tx and rx maps into a single map.
	IterableTools it;
	output_chanid_remoteaddr_map = it.CombineCompoundMapsToSet(
		chanid_remoteaddr_map1, chanid_remoteaddr_map2);
}

void ParseManager::collect_chanid_to_commwords_metadata(
	std::map<uint32_t, std::vector<std::vector<uint32_t>>>& output_chanid_commwords_map)
{
	// Collect maps into one.
	std::map<uint32_t, std::set<uint32_t>> chanid_commwords_map;
	for (uint16_t read_ind = 0; read_ind < n_reads; read_ind++)
	{
		workers[read_ind].append_chanid_comwmwords_map(chanid_commwords_map);
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

std::streamsize ParseManager::activate_worker(uint16_t binbuff_ind, uint16_t ID,
	uint64_t start_pos, uint32_t n_read)
{
	uint64_t read_count = binary_buffers[binbuff_ind].Initialize(ifile, total_size, start_pos, n_read);
#ifdef PARQUET
	bool is_final_worker = false;
	if (ID == n_reads - 1)
	{
		is_final_worker = true;
	}
	workers[ID].initialize(ID, start_pos, n_read, binbuff_ind, output_file_path_vec_[ID],
		is_final_worker);
#endif

	spdlog::get("pm_logger")->debug("Init. worker {:d}: start = {:d}, read size = {:d}, bb ind = {:d}",
		ID, start_pos, n_read, binbuff_ind);
		
	// Start this instance of ParseWorker. Append mode false.
#if defined PARSER_REWRITE
	threads[ID] = std::thread(std::ref(workers[ID]), std::ref(binary_buffers[binbuff_ind]),
		false, std::ref(tmats_body_vec_), packet_type_config_map_);
#elif defined LIBIRIG106
	threads[ID] = std::thread(std::ref(workers[ID]), std::ref(binary_buffers[binbuff_ind]),
		false, std::ref(tmats_body_vec_));
#else
	threads[ID] = std::thread(std::ref(workers[ID]), std::ref(binary_buffers[binbuff_ind]),
		false);
#endif
	
	return read_count;
}

std::streamsize ParseManager::activate_append_mode_worker(uint16_t binbuff_ind, uint16_t ID,
	uint32_t n_read)
{
	// Reset parse worker completion status.
	uint64_t last_pos = workers[ID].get_last_position();
	workers[ID].reset_completion_status();

	spdlog::get("pm_logger")->debug("Init. append mode worker {:d}: start = {:d}, read size = {:d}, bb ind = {:d}",
		ID, last_pos, n_read, binbuff_ind);

	uint64_t read_count = binary_buffers[binbuff_ind].Initialize(ifile,
		total_size, last_pos, n_read);

	workers[ID].append_mode_initialize(n_read, binbuff_ind, last_pos);

	// Start this instance of ParseWorker. Append mode true.
#if defined PARSER_REWRITE
	threads[ID] = std::thread(std::ref(workers[ID]), std::ref(binary_buffers[binbuff_ind]),
		true, std::ref(tmats_body_vec_), packet_type_config_map_);
#elif defined LIBIRIG106
	threads[ID] = std::thread(std::ref(workers[ID]), std::ref(binary_buffers[binbuff_ind]),
		true, std::ref(tmats_body_vec_));
#else
	threads[ID] = std::thread(std::ref(workers[ID]), std::ref(binary_buffers[binbuff_ind]),
		true);
#endif
	return read_count;
}

void ParseManager::worker_queue(bool append_mode)
{
	// Initially load all threads by keeping track of the
	// the number activated. Afterwards only activate new 
	// threads when an active thread finishes.
	uint16_t active_thread_count = 0;
	uint16_t current_active_worker = 0;
	bool thread_started = false;
	bool all_threads_active = false;
	bool eof_reached = false;
	uint16_t max_worker_ind = 0;
	std::streamsize current_read_count = 0;
	uint16_t bb_ind = 0;
	int concurrent_thread_count = 0;
	spdlog::get("pm_logger")->debug("worker_queue: Starting worker threads");

	// Create and wait for workers until the entire input file 
	// is parsed.

	// Loop over each block of data that is to be read and parsed. There is a 1:1
	// relationship between a worker and a block of data--each worker consumes a 
	// single data block. In the append_mode = true
	// case, the very last worker doesn't have any dangling packets because presumably
	// it parsed data until the ch10 file ended. Only [worker 1, worker last) need to
	// append data to their respective files. 
	uint16_t n_read_limit = n_reads;
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
			return;
	}
	for (uint16_t read_ind = 0; read_ind < n_read_limit; read_ind++)
	{
		thread_started = false;
		// Stay in while loop until another worker is started. 
		while (!thread_started)
		{
			if (!all_threads_active)
			{
				spdlog::get("pm_logger")->debug("All threads NOT ACTIVE ({:d} active)", active_thread_count);

				// Immediately activate a new worker.
				if (append_mode)
				{
					//printf("Ready to activate_append_mode_worker, active_thread_count = %hu, read_ind = %hu\n", active_thread_count, read_ind);
					current_read_count = activate_append_mode_worker(active_thread_count,
						read_ind, append_read_size);
				}
				else
				{
					current_read_count = activate_worker(active_thread_count, read_ind, total_read_pos,
						read_size);
				}

				// Check if the correct number of bytes were read.
				if (append_mode)
				{
					if (current_read_count != append_read_size)
					{
						eof_reached = true;
						spdlog::get("pm_logger")->debug("Current read count {:d}, not equal to expected size {:d}", 
							current_read_count, append_read_size);
					}
				}
				else if (current_read_count != read_size)
				{
					eof_reached = true;
					spdlog::get("pm_logger")->debug("EOF reached: {:d}/{:d} read", 
						current_read_count, read_size);
				}

				// Put worker in active workers list.
				active_workers.push_back(read_ind);
				
				//max_worker_ind = read_ind;

				active_thread_count += 1;
				if (active_thread_count == n_threads)
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
				if(!append_mode && active_thread_count != n_reads)
					std::this_thread::sleep_for(worker_start_offset);
			}
			else
			{
				spdlog::get("pm_logger")->debug("All threads ACTIVE ({:d} active)", active_thread_count);
				// Check active workers to see if they are ready to be joined.
				for (size_t act_work_ind = 0; act_work_ind < active_workers.size(); act_work_ind++)
				{
					
					// Join workers that are complete
					if (workers[active_workers[act_work_ind]].completion_status() == true)
					{
						spdlog::get("pm_logger")->debug("Worker {:d} INACTIVE/COMPLETE -- joining now", 
							active_workers[act_work_ind]);

						bb_ind = workers[active_workers[act_work_ind]].get_binbuff_ind();
						threads[active_workers[act_work_ind]].join();
						active_workers.erase(active_workers.begin() + act_work_ind);

						// Immediately start a new worker.
						if (append_mode)
						{
							current_read_count = activate_append_mode_worker(bb_ind,
								read_ind, append_read_size);
						}
						else
						{
							current_read_count = activate_worker(bb_ind, read_ind, total_read_pos,
								read_size);
						}

						// Check if the correct number of bytes were read.
						if (append_mode)
						{
							if (current_read_count != append_read_size)
							{
								spdlog::get("pm_logger")->debug(
									"Worker {:d} Error: {:d} bytes read, {:d} bytes indicated",
									read_ind, current_read_count, append_read_size);
							}
						}
						else if (current_read_count != read_size)
						{
							eof_reached = true;
							spdlog::get("pm_logger")->debug("EOF reached: {:d}/{:d} read", 
								current_read_count, read_size);
						}

						// Place new worker among active workers.
						active_workers.push_back(read_ind);

						thread_started = true;
						break;
					}
					else
					{
						spdlog::get("pm_logger")->debug("Worker {:d} STILL ACTIVE", 
							active_workers[act_work_ind]);
					}

				}
			}

			// Wait before checking for available workers.
			spdlog::get("pm_logger")->trace("Waiting for workers");
			std::this_thread::sleep_for(worker_wait);

		} // end while 

		// Increase the total read position.
		total_read_pos += current_read_count;

		if (eof_reached)
			break;

	} // end for loop over all worker indices.
}

void ParseManager::worker_retire_queue()
{
	spdlog::get("pm_logger")->debug("Joining all remaining workers");
	while (active_workers.size() > 0)
	{
		for (size_t act_work_ind = 0; act_work_ind < active_workers.size(); act_work_ind++)
		{
			// Join workers that are complete
			if (workers[active_workers[act_work_ind]].completion_status() == true)
			{
				spdlog::get("pm_logger")->debug("Worker {:d} INACTIVE/COMPLETE -- joining now", 
					active_workers[act_work_ind]);
			
				threads[active_workers[act_work_ind]].join();

				spdlog::get("pm_logger")->debug("Worker {:d} joined", active_workers[act_work_ind]);
				if (n_reads == 1)
					active_workers.resize(0);
				else
					active_workers.erase(active_workers.begin() + act_work_ind);
			}
			else
			{
				spdlog::get("pm_logger")->debug("Worker {:d} STILL ACTIVE", active_workers[act_work_ind]);
			}
		}

		// Wait before checking for available workers.
		spdlog::get("pm_logger")->debug("Waiting for workers to complete");
		std::this_thread::sleep_for(worker_wait);
	}

	spdlog::get("pm_logger")->debug("All workers joined ");
}

void ParseManager::collect_stats()
{
	PacketStats pstats;
	PacketStats pstats_error;
	PacketStats* pstats_temp;
	//Ch10MilStd1553F1Stats milstdstats;
	Ch10MilStd1553F1Stats* milstdstats = workers[0].milstd1553_stats();

	// Loop over all workers and collect stats from each.
	for (uint16_t worker_ind = 0; worker_ind < n_reads; worker_ind++)
	{
		// Get present workers packet stats and sum with final count.
		pstats_temp = workers[worker_ind].get_packet_ledger();
		pstats.time_data_count += pstats_temp->time_data_count;
		pstats.milstd_1553pkt_count += pstats_temp->milstd_1553pkt_count;
		pstats.video_data_count += pstats_temp->video_data_count;
		pstats.discrete_data_count += pstats_temp->discrete_data_count;
		pstats.analog_data_count += pstats_temp->analog_data_count;

		pstats_temp = workers[worker_ind].get_packet_error_ledger();
		pstats_error.time_data_count += pstats_temp->time_data_count;
		pstats_error.milstd_1553pkt_count += pstats_temp->milstd_1553pkt_count;
		pstats_error.video_data_count += pstats_temp->video_data_count;
		pstats_error.discrete_data_count += pstats_temp->discrete_data_count;
		pstats_error.analog_data_count += pstats_temp->analog_data_count;

		// Get present workers mil std 1553 message stats.
		if (worker_ind > 0)
			milstdstats->add_stats(workers[worker_ind].milstd1553_stats());
		/*milstdstats_temp = workers[worker_ind].milstd1553_stats();
		milstdstats.msg_identity_fail += milstdstats_temp->msg_identity_fail;
		milstdstats.wrd_count_mismatch += milstdstats_temp->wrd_count_mismatch;
		milstdstats.ts_not_impl += milstdstats_temp->ts_not_impl;
		milstdstats.ts_reserved += milstdstats_temp->ts_reserved;*/
	}

	#ifdef DEBUG
	if (DEBUG > 0)
	{
		uint32_t err = 0;
		uint32_t noerr = 0;
		err = pstats_error.time_data_count;
		noerr = pstats.time_data_count;
		printf("\n-- Packet Type Stats (err/total : %%) --\n");
		printf("Time data          : %02u/%05u : %f\n", err, err + noerr, float(err) / (err + noerr));

		err = pstats_error.milstd_1553pkt_count;
		noerr = pstats.milstd_1553pkt_count;
		printf("MilStd 1553        : %02u/%05u : %f\n", err, err + noerr, float(err) / (err + noerr));

		err = pstats_error.video_data_count;
		noerr = pstats.video_data_count;
		printf("Video data         : %02u/%05u : %f\n", err, err + noerr, float(err) / (err + noerr));

		err = pstats_error.discrete_data_count;
		noerr = pstats.discrete_data_count;
		printf("Discrete data      : %02u/%05u : %f\n", err, err + noerr, float(err) / (err + noerr));

		err = pstats_error.analog_data_count;
		noerr = pstats.analog_data_count;
		printf("Analog data        : %02u/%05u : %f\n", err, err + noerr, float(err) / (err + noerr));

		printf("\n-- MilStd 1553 Error Stats --\n");
		printf("MSG_IDENTITY_FAIL  : %03u\n", milstdstats->msg_identity_fail);
		printf("WRD_COUNT_MISMATCH : %03u\n", milstdstats->wrd_count_mismatch);
		printf("TS_NOT_IMPL        : %03u\n", milstdstats->ts_not_impl);
		printf("TS_RESERVED        : %03u\n\n", milstdstats->ts_reserved);
	}
	#endif

	milstdstats->print_msg_stats();
}

ParseManager::~ParseManager()
{
	ifile.close();
	if(workers_allocated)
	{
		spdlog::get("pm_logger")->debug(
			"ParseManager: Deleting \"workers\", \"binary_buffers\", \"threads\"");
		
		delete[] threads;
		delete[] workers;
		delete[] binary_buffers;
	}
}

void ParseManager::ProcessTMATS()
{
	// if tmats doesn't exist return
	if (tmats_body_vec_.size() == 0)
	{
		spdlog::get("pm_logger")->warn("No TMATS Present");
		return;
	}

	std::string full_TMATS_string;
	for (int i = 0; i < tmats_body_vec_.size(); i++)
	{
		full_TMATS_string += tmats_body_vec_[i];
	}

	ManagedPath tmats_path;
	tmats_path = output_dir_map_[Ch10DataType::MILSTD1553_DATA_F1] /
		"_TMATS.txt";
	std::ofstream tmats;
	tmats.open(tmats_path.string(), std::ios::trunc | std::ios::binary);
	if (tmats.good())
	{
		spdlog::get("pm_logger")->info("Writing TMATS to {:s}", tmats_path.RawString());
		tmats << full_TMATS_string;
	}

	tmats.close();

	// Gather TMATs attributes of interest
	// for metadata
	TMATSParser tmats_parser = TMATSParser(full_TMATS_string);
	TMATsChannelIDToSourceMap_ = tmats_parser.MapAttrs("R-x\\TK1-n", "R-x\\DSI-n");
	TMATsChannelIDToTypeMap_ = tmats_parser.MapAttrs("R-x\\TK1-n", "R-x\\CDT-n");
}

#ifdef PARSER_REWRITE
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
	for (std::map<Ch10PacketType, bool>::const_iterator it = packet_type_config_map_.cbegin();
		it != packet_type_config_map_.cend(); ++it)
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
#endif
