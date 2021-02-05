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
#ifdef DEBUG
#if DEBUG > 0
		printf("File size: %llu MB\n\n", total_size / (1024 * 1024));
#endif
#endif
		create_output_dirs();
	}
	else
		error_set = true;
}

void ParseManager::create_output_dirs()
{
	ManagedPath parquet_1553_path = output_path.CreatePathObject(input_path, "_1553.parquet");
	printf("Parquet 1553 output path: %s\n", parquet_1553_path.RawString().c_str());
	if (!parquet_1553_path.create_directory())
		error_set = true;
	output_dir_map_[Ch10DataType::MILSTD1553_DATA_F1] = parquet_1553_path;

#ifdef VIDEO_DATA

	ManagedPath parquet_vid_path = output_path.CreatePathObject(input_path, "_video.parquet");
	printf("Parquet video output path: %s\n", parquet_vid_path.RawString().c_str());
	if (!parquet_vid_path.create_directory())
		error_set = true;
	output_dir_map_[Ch10DataType::VIDEO_DATA_F0] = parquet_vid_path;

#endif

#ifdef ETHERNET_DATA

	ManagedPath parquet_eth_path = output_path.CreatePathObject(input_path, "_ethernet.parquet");
	printf("Parquet ethernet output path: %s\n", parquet_eth_path.RawString().c_str());
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
		printf("Error opening file: %s\n", input_path.RawString().c_str());
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

	printf("\nInput file: %s\n", input_path.RawString().c_str());
	printf("Using %u threads\n", n_threads);
#ifdef DEBUG
	if (DEBUG > 0)
	{
		printf("Created %u workers\n", n_reads);
		printf("Created %u binary buffers\n", n_reads);
	}
#endif	

	// Start queue to activate all workers, limiting the quantity of 
	// concurrent threads to n_threads.
	bool append = false;
	worker_queue(append);
	printf("after worker_queue\n");

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

#ifdef DEBUG
	if (DEBUG > 0)
		printf("\n-- Parsing dangling packets --\n");
#endif
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




#ifdef LIBIRIG106
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

	#ifdef DEBUG
	if (DEBUG > 1)
	{
		printf("Init. worker %u: start = %llu, read size = %u, bb ind = %u\n", 
			ID, start_pos, n_read, binbuff_ind);
	}
	#endif
		
	// Start this instance of ParseWorker. Append mode false.
#if defined PARSER_REWRITE
	threads[ID] = std::thread(std::ref(workers[ID]), std::ref(binary_buffers[binbuff_ind]),
		false);
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

	#ifdef DEBUG
	if (DEBUG > 1)
	{
		printf("Init. append mode worker %u: start = %llu, read size = %u, bb ind = %u\n",
			ID, last_pos, n_read, binbuff_ind);
	}
	#endif

	uint64_t read_count = binary_buffers[binbuff_ind].Initialize(ifile,
		total_size, last_pos, n_read);

	workers[ID].append_mode_initialize(n_read, binbuff_ind, last_pos);

	// Start this instance of ParseWorker. Append mode true.
#if defined PARSER_REWRITE
	threads[ID] = std::thread(std::ref(workers[ID]), std::ref(binary_buffers[binbuff_ind]),
		true);
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

	#ifdef DEBUG
	if (DEBUG > 0)
		printf("Starting worker threads\n");
	#endif

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
				#ifdef DEBUG
				if (DEBUG > 1)
					printf("\nAll threads NOT ACTIVE (%u active).\n", active_thread_count);
				#endif

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
						printf("Current read count %Iu, not equal to expected size %u\n", current_read_count, append_read_size);
					}
				}
				else if (current_read_count != read_size)
				{
					eof_reached = true;
					#ifdef DEBUG
					if (DEBUG > 1)
						printf("EOF reached: %Iu/%u read\n", current_read_count, read_size);
					#endif
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
				#ifdef DEBUG
				if (DEBUG > 1)
					printf("\nAll threads ACTIVE (%u active).\n", active_thread_count);
				#endif
				// Check active workers to see if they are ready to be joined.
				for (size_t act_work_ind = 0; act_work_ind < active_workers.size(); act_work_ind++)
				{
					
					// Join workers that are complete
					if (workers[active_workers[act_work_ind]].completion_status() == true)
					{
						#ifdef DEBUG
						if (DEBUG > 1)
							printf("Worker %u INACTIVE/COMPLETE -- joining now\n", active_workers[act_work_ind]);

						#endif

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
								printf("Worker %hu Error: %Iu bytes read, %u bytes indicated\n",
									read_ind, current_read_count, append_read_size);
							}
						}
						else if (current_read_count != read_size)
						{
							eof_reached = true;
							#ifdef DEBUG
							if (DEBUG > 1)
								printf("EOF reached: %Iu/%u read\n", current_read_count, read_size);
							#endif
						}

						// Place new worker among active workers.
						active_workers.push_back(read_ind);
						//max_worker_ind = read_ind;

						thread_started = true;
						break;
					}
					else
					{
						#ifdef DEBUG
						if (DEBUG > 1)
							printf("Worker %u STILL ACTIVE\n", active_workers[act_work_ind]);
						#endif
					}

				}
			}

			// Wait before checking for available workers.
			#ifdef DEBUG
			if (DEBUG > 1)
				printf("Waiting for workers\n");
			#endif
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
	#ifdef DEBUG
	if (DEBUG > 0)
		printf("\n-- Joining all remaining workers --\n");
	#endif
	while (active_workers.size() > 0)
	{
		for (size_t act_work_ind = 0; act_work_ind < active_workers.size(); act_work_ind++)
		{
			// Join workers that are complete
			if (workers[active_workers[act_work_ind]].completion_status() == true)
			{
				#ifdef DEBUG
				if (DEBUG > 0)
					printf("Worker %hu INACTIVE/COMPLETE -- joining now\n", active_workers[act_work_ind]);
				#endif
			
				threads[active_workers[act_work_ind]].join();

#ifdef PARQUET
#endif

				#ifdef DEBUG
				if (DEBUG > 2)
				{
					printf("Worker %u joined\n", active_workers[act_work_ind]);
				}
				#endif
				if (n_reads == 1)
					active_workers.resize(0);
				else
					active_workers.erase(active_workers.begin() + act_work_ind);
			}
			else
			{
				#ifdef DEBUG
				if (DEBUG > 1)
					printf("Worker %u STILL ACTIVE\n", active_workers[act_work_ind]);
				#endif
			}
		}

		// Wait before checking for available workers.
		#ifdef DEBUG
		if (DEBUG > 1)
			printf("Waiting for workers to complete\n");
		#endif
		std::this_thread::sleep_for(worker_wait);
	}

	#ifdef DEBUG
	if (DEBUG > 0)
		printf("\n-- All workers joined --\n");
	#endif
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
	//printf("in destructor ... \n");
	if(workers_allocated)
	{
		#ifdef DEBUG
		if (DEBUG > 1)
			printf("ParseManager: Deleting \"workers\", \"binary_buffers\", \"threads\"\n");
		#endif
		
		delete[] threads;
		//printf("after delete threads\n");
		delete[] workers;
		//printf("after delete workers\n");
		delete[] binary_buffers;
		//printf("after delete binary_buffers\n");
		
	}
}

void ParseManager::ProcessTMATS()
{
	// if tmats doesn't exist return
	if (tmats_body_vec_.size() == 0)
	{
		printf("\nNo TMATS Present\n");
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
		printf("\nWriting TMATS to %s\n", tmats_path.RawString().c_str());
		tmats << full_TMATS_string;
	}

	tmats.close();

	// Gather TMATs attributes of interest
	// for metadata
	TMATSParser tmats_parser = TMATSParser(full_TMATS_string);
	TMATsChannelIDToSourceMap_ = tmats_parser.MapAttrs("R-x\\TK1-n", "R-x\\DSI-n");
	TMATsChannelIDToTypeMap_ = tmats_parser.MapAttrs("R-x\\TK1-n", "R-x\\CDT-n");
}
