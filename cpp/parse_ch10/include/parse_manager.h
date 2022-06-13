// parse_manager.h

/* ParseManager is called by the main() ch10parse entry point.
   It has functions and data members for managing multiple threads
   which in turn are each represented by a ParseWorker instance.
*/

#ifndef PARSEMANAGER_H
#define PARSEMANAGER_H

#include <cstdint>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <memory>
#include <map>
#include <set>

#include "parse_text.h"
#include "parse_worker.h"
#include "parser_config_params.h"
#include "parser_paths.h"
#include "managed_path.h"
#include "worker_config.h"
#include "spdlog/spdlog.h"
#include "ch10_packet_type.h"
#include "ch10_context.h"
#include "parquet_tdpf1.h"
#include "parser_metadata.h"

class ParseManager
{
   private:
    // Total ch10 file size, bytes
    uint64_t ch10_file_size_;

    // TMATS raw data
    std::vector<std::string> tmats_body_vec_;

    // Count of bytes of raw ch10 data to be parsed by each
    // worker
    uint64_t worker_chunk_size_bytes_;

    // Count of bytes of raw ch10 data to be parsed by
    // each worker in append mode
    uint32_t append_chunk_size_bytes_;

    // Count of workers necessary to parse the entire ch10
    // based on worker_chunk_size_bytes_ and the total
    // file size.
    uint16_t worker_count_;

	// Context instances are paired with each ParseWorker
	std::vector<std::unique_ptr<Ch10Context>> context_vec_;

    // Workers necessary to parse the ch10 based on user
    // configuration. Use unique_ptr to avoid creating a copy
    // assignment operator for ParseWorker and Ch10Context.
    std::vector<std::unique_ptr<ParseWorker>> workers_vec_;

    // One thread in which each worker can execute
    std::vector<std::thread> threads_vec_;

    // One WorkerConfig for each worker
    std::vector<WorkerConfig> worker_config_vec_;

    // Read ch10 binary data
    std::ifstream ch10_input_stream_;

	// Create and manage paths relevant to the parser
	ParserPaths parser_paths_;

	// Handle all parse metadata related activities
	ParserMetadata parser_metadata_;

   public:
	static const std::string metadata_filename_;
    const uint64_t& worker_chunk_size_bytes;
    const uint16_t& worker_count;
    const std::vector<std::unique_ptr<ParseWorker>>& workers_vec;
    const std::vector<std::thread>& threads_vec;
    const std::vector<WorkerConfig>& worker_config_vec;

    ParseManager();
    virtual ~ParseManager();

    /*
	** High-level function which coordinates lower-level functions. Intended
	** to be called first by the user.

	Collect misc setup functions and validate configuration.

	Args:
		input_ch10_file_path	--> Ch10 file to parse
		output_dir				--> Directory in which to place
									parsed data
		config					--> ParserConfigParams object which has
									been pre-configured with data from
									the parser_conf.yaml file

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
    virtual bool Configure(ManagedPath input_ch10_file_path, ManagedPath output_dir,
                   const ParserConfigParams& user_config);

    /*
	** High-level function which coordinates lower-level functions. Intended
	** to be called by the user after Configure.

	Parse the ch10 file and record data to the configured file type.

	Args:
		config					--> ParserConfigParams object which has
									been pre-configured with data from
									the parser_conf.yaml file

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
   	virtual bool Parse(const ParserConfigParams& user_config);

    /*
	**High-level function which coordinates lower-level functions. Intended
	**to be called by the user after Parse.

	Finalize the parse process by recording metadata.

	Args:
		input_ch10_file_path	--> Ch10 file that was parsed
		config					--> ParserConfigParams object which has
									been pre-configured with data from
									the parser_conf.yaml file

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
    virtual bool RecordMetadata(ManagedPath input_ch10_file_path,
                        const ParserConfigParams& user_config);

    //////////////////////////////////////////////////////////////////////////////
    // Functions below are considered to be internal functions. They
    // are made public to facilitate testing.
    //////////////////////////////////////////////////////////////////////////////



    /*
	Initialize parameters in the relevant WorkerConfig object in preparation
	for parsing by a worker.

	Args:
		worker_config		--> WorkerConfig object which will be configured
								for the relevant ParseWorker
		worker_index		--> Index of the objects in worker_vecs_, threads_vec_,
								and worker_config_vec_ which are being configured
								for parsing
		worker_count		--> Total count of workers, also equal to .size() of
								worker_vecs_, threads_vec_, and worker_config_vec_
		read_pos			--> Position in bytes from which ch10 is read into
								the buffer in preparation for parsing
		read_size			--> Size of chunk in bytes of ch10 to parse for each
								first-pass worker
		total_size			--> Total size of the ch10 in bytes
		binbuff_ptr			--> Pointer to buffer object into which data shall
								be read for this worker to consume
		ch10_input_stream	--> Initialized input stream for the ch10 file to
								be parsed
		actual_read_size	--> Output variable to hold the size of bytes actually
								read into the buffer
		output_file_path_vec--> Output file path for each configured Ch10PacketType
		packet_type_config_map--> Map of Ch10PacketType to boolean. True = enabled,
								  False = disabled. This map is created from the
								  'ch10_packet_type' map in the parse_conf.yaml.

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
    bool ConfigureWorker(WorkerConfig& worker_config, const uint16_t& worker_index,
                         const uint16_t& worker_count, const uint64_t& read_pos, const uint64_t& read_size,
                         const uint64_t& total_size, BinBuff* binbuff_ptr,
                         std::ifstream& ch10_input_stream, std::streamsize& actual_read_size,
                         const std::map<Ch10PacketType, ManagedPath>& output_file_path_map,
                         const std::map<Ch10PacketType, bool>& packet_type_config_map);

    /*
	Initialize parameters in the relevant WorkerConfig object in preparation
	for parsing in append mode by a worker.

	Args:
		worker_config		--> WorkerConfig object which will be configured
								for the relevant ParseWorker
		worker_index		--> Index of the objects in worker_vecs_, threads_vec_,
								and worker_config_vec_ which are being configured
								for parsing
		append_read_size	--> Size of chunk in bytes of ch10 to parse for each
								append-mode worker

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
    bool ConfigureAppendWorker(WorkerConfig& worker_config, const uint16_t& worker_index,
                               const uint64_t& append_read_size, const uint64_t& total_size, BinBuff* binbuff_ptr,
                               std::ifstream& ch10_input_stream, std::streamsize& actual_read_size);

    /*
	Configure and activate worker to parse a chunk of the ch10.

	Not tested due to difficulty of mocking std::thread. Utilizes
	tested functions.

	Args:
		append_mode			--> True if append_mode workers are to be started,
								false otherwise
		parse_worker_ptr	--> Pointer to ParseWorker object in which to
								parse the chunk indicated by other args to this
								function
		worker_thread		--> Thread in which to execute the ParseWorker
		worker_config		--> WorkerConfig object which will be configured
								for the relevant ParseWorker
		worker_index		--> Index of the objects in worker_vecs_, threads_vec_,
								and worker_config_vec_ which are being configured
								for parsing
		worker_count		--> Total count of workers, also equal to .size() of
								worker_vecs_, threads_vec_, and worker_config_vec_
		read_pos			--> Position in bytes from which ch10 is read into
								the buffer in preparation for parsing
		read_size			--> Size of chunk in bytes of ch10 to parse for each
								first-pass worker
		append_read_size	--> Size of chunk in bytes of ch10 to parse for each
								append-mode worker
		total_size			--> Total size of the ch10 in bytes
		binbuff_ptr			--> Pointer to buffer object into which data shall
								be read for this worker to consume
		ch10_input_stream	--> Initialized input stream for the ch10 file to
								be parsed
		actual_read_size	--> Output variable to hold the size of bytes actually
								read into the buffer
		output_file_path_vec--> Output file path for each configured Ch10PacketType
		packet_type_config_map--> Map of Ch10PacketType to boolean. True = enabled,
								  False = disabled.
		tmats_vec			--> Vector of strings into which workers will push
								TMATS matter
		ctx					--> Ch10Context pointer, specific to the worker being
								activated

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
    bool ActivateWorker(bool append_mode, std::unique_ptr<ParseWorker>& parse_worker_ptr,
                        std::thread& worker_thread, WorkerConfig& worker_config, const uint16_t& worker_index,
                        const uint16_t& worker_count, const uint64_t& read_pos, const uint64_t& read_size,
                        const uint64_t& append_read_size, const uint64_t& total_size, BinBuff* binbuff_ptr,
                        std::ifstream& ch10_input_stream, std::streamsize& actual_read_size,
                        const std::map<Ch10PacketType, ManagedPath>& output_file_path_map,
                        const std::map<Ch10PacketType, bool>& packet_type_config_map,
                        std::vector<std::string>& tmats_vec, std::unique_ptr<Ch10Context>& ctx);

    /*
	Start workers in a queue in quantity up to the user-configured thread
	count. Wait for available threads and start additional workers until
	worker_count workers have run their course. Differentiate between
	first-pass and append-mode workers.

	Args:
		append_mode			--> True if append_mode workers are to be started,
								false otherwise
		ch10_input_stream	--> Initialized input stream for the ch10 file to
								be parsed
		worker_vec			--> Vector of ParseWorker pointers, each of which
								parse a chunk of the ch10.
		active_workers_vec	--> Track the index of active workers
		worker_config_vec	--> Worker configuration objects which are associated
								by index to worker_vec ParseWorkers. Must have size()
								equal to worker_vec.size().
		effective_worker_count> Maximum count of workers to start. Useful for
								append_mode = true, in which case there is one
								fewer worker because the last worker reached the
								end of the file so there is no append mode.
		read_size			--> Size of chunk in bytes of ch10 to parse for each
								first-pass worker
		append_read_size	--> Size of chunk in bytes of ch10 to parse for each
								append-mode worker
		total_size			--> Total size of the ch10 in bytes
		output_file_path_vec--> Vector of maps in which the index in
								the vector is the same as the worker which
								ought to utilize the mapped output file paths.
		packet_type_config_map--> Map of Ch10PacketType to boolean. True = enabled,
								  False = disabled.
		threads_vec			--> Vector of std::thread objects. Must have size()
								equal to worker_vec.size().
		tmats_vec			--> Vector of strings into which workers will push
								TMATS matter
		user_config			--> ParserConfigParams object which has been
								pre-configured with data from the
								parser_conf.yaml file

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.

	*/
    bool WorkerQueue(bool append_mode, std::ifstream& ch10_input_stream,
                     std::vector<std::unique_ptr<ParseWorker>>& worker_vec,
                     std::vector<uint16_t>& active_workers_vec,
                     std::vector<WorkerConfig>& worker_config_vec,
                     const uint16_t& effective_worker_count,
                     const uint64_t& read_size, const uint64_t& append_read_size,
                     const uint64_t& total_size,
                     const std::vector<std::map<Ch10PacketType, ManagedPath>>& output_file_path_vec,
                     const std::map<Ch10PacketType, bool>& packet_type_config_map,
                     std::vector<std::thread>& threads_vec,
                     std::vector<std::string>& tmats_vec,
                     const ParserConfigParams& user_config);

    /*
	Wait for each worker to finish before returning. As each worker completes
	the parsing job asynchronously, join the thread and deallocate the buffer.

	Args:
	worker_vec				--> Vector of ParseWorker pointers, each of which
								parse a chunk of the ch10.
	active_workers_vec		--> Track the index of active workers
	worker_config_vec		--> Worker configuration objects which are associated
								by index to worker_vec ParseWorkers. Must have size()
								equal to worker_vec.size().
	threads_vec				--> Vector of std::thread objects. Must have size()
								equal to worker_vec.size().
	worker_shift_wait		--> Main thread sleep duration in loop after
								checking for any completed workers, millisecond

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
    bool WorkerRetireQueue(std::vector<std::unique_ptr<ParseWorker>>& worker_vec,
                           std::vector<uint16_t>& active_workers_vec,
                           std::vector<WorkerConfig>& worker_config_vec,
                           std::vector<std::thread>& threads_vec,
                           int worker_shift_wait);

    /*
	Log the ch10_packet_type_map_. This is a convenience function to clean up
	the clutter that this code introduces.

	Args:
		pkt_type_config_map	--> Input map of Ch10PacketType to bool that
								represents the enable state of ch10 packet types
	*/
    void LogPacketTypeConfig(const std::map<Ch10PacketType, bool>& pkt_type_config_map);


    /*
	Calculate paramaters from user configuration and do initial
	vector allocation.

	Args:
		user_config			--> ParserConfigParams object which has been
								pre-configured with data from the
								parser_conf.yaml file
		ch10_file_size		--> Total size of the ch10 in bytes

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
    bool AllocateResources(const ParserConfigParams& user_config,
                           const uint64_t& ch10_file_size);



    /*
	Collect raw TMATS strings into a single string and write to disk.
	Use TMATSParser to parse and associate TMATS metadata into maps
	in preparation for recording to yaml metadata file.

	Args:
	tmats_vec					--> Vector of strings into which workers will push
									TMATS matter
	tmats_file_path				--> Complete path including the output file name
									to which TMATS matter is recorded
	tmats_data					--> TMATSData object
	parsed_pkt_types			--> Set of Ch10PacketType that contains only the
									present and parsed types

	*/
    void ProcessTMATS(const std::vector<std::string>& tmats_vec,
                      const ManagedPath& tmats_file_path,
					  TMATSData& tmats_data, 
					  const std::set<Ch10PacketType>& parsed_pkt_types);

	
	
	/*
	Assemble the set of all parsed packet types from the workers and
	log the information.

	Args:
		parsed_packet_types 	--> Set of Ch10PacketType to be filled
	*/
	void AssembleParsedPacketTypesSet(std::set<Ch10PacketType>& parsed_packet_types);



	/*
	Record time data ("Time Data, Format 1") which is stored in Ch10Context
	instances to Parquet format.

	Args:
		ctx_vec			--> Vector of Ch10Contexts from which time data
							will be gathered
		pqtdp			--> Instance of ParquetTDPF1 which will be used
							to write the time data to Parquet
		file_path		--> Complete output file path

	Return:
		False if any steps fail; true otherwise.
	*/
	bool WriteTDPData(const std::vector<const Ch10Context*>& ctx_vec,
		ParquetTDPF1* pqtdp, const ManagedPath& file_path);
};

#endif