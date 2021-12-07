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
#include "iterable_tools.h"
#include "parse_text.h"
#include "parse_worker.h"
#include "parser_config_params.h"
#include "tmats_parser.h"
#include "managed_path.h"
#include "provenance_data.h"
#include "tip_md_document.h"
#include "sha256_tools.h"
#include "worker_config.h"
#include "spdlog/spdlog.h"
#include "ch10_packet_type.h"

class ParseManager
{
   private:
    // Total ch10 file size, bytes
    uint64_t ch10_file_size_;

    // TMATS raw data
    std::vector<std::string> tmats_body_vec_;

    // Metadata manipulation
    IterableTools it_;

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

    // Base output directory per Ch10PacketType
    std::map<Ch10PacketType, ManagedPath> output_dir_map_;

    // Configured file paths. The worker at index x in workers_vec_ shall
    // create output files according to the paths in the map at index x
    // in this vector.
    std::vector<std::map<Ch10PacketType, ManagedPath>> output_file_path_vec_;

   public:
    const uint64_t& worker_chunk_size_bytes;
    const uint16_t& worker_count;
    const std::vector<std::unique_ptr<ParseWorker>>& workers_vec;
    const std::vector<std::thread>& threads_vec;
    const std::vector<WorkerConfig>& worker_config_vec;

    ParseManager();
    ~ParseManager();

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
    bool Configure(ManagedPath input_ch10_file_path, ManagedPath output_dir,
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
    bool Parse(const ParserConfigParams& user_config);

    /*
	**High-level function which coordinates lower-level functions. Intended
	**to be called by the user after Parse.

	Finalize the parse process by recording metadata.

	Args:
		input_ch10_file_path	--> Ch10 file that was parsed
		config					--> ParserConfigParams object which has
									been pre-configured with data from
									the parser_conf.yaml file
		prov_data				--> Object of ProvenanceData that contains
									pre-filled provenance data

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
    bool RecordMetadata(ManagedPath input_ch10_file_path,
                        const ParserConfigParams& user_config,
						const ProvenanceData& prov_data);

    //////////////////////////////////////////////////////////////////////////////
    // Functions below are considered to be internal functions. They
    // are made public to facilitate testing.
    //////////////////////////////////////////////////////////////////////////////

    /*
	Record metadata specific to MilStd 1553 Format 1

	Args:
		input_ch10_file_path	--> Ch10 file that was parsed
		config					--> ParserConfigParams object which has
									been pre-configured with data from
									the parser_conf.yaml file
		prov_data				--> Object of ProvenanceData that contains
									pre-filled provenance data
		tmats_chanid_source		--> Prepared TMATS channel ID to source map
		tmats_chanid_type		--> Prepared TMATS channel ID to type map
		packet_type_label		--> See ch10_packet_type.h
		md_file_path			--> Output directory and file name for metadata

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
    bool RecordMilStd1553F1Metadata(ManagedPath input_ch10_file_path,
                                    const ParserConfigParams& user_config,
									const ProvenanceData& prov_data,
									const std::map<std::string, std::string> tmats_chanid_source,
									const std::map<std::string, std::string> tmats_chanid_type,
									const std::string& packet_type_label,
									const ManagedPath& md_file_path);

    /*
	Record metadata specific to Video data format 0

	Args:
		input_ch10_file_path	--> Ch10 file that was parsed
		config					--> ParserConfigParams object which has
									been pre-configured with data from
									the parser_conf.yaml file
		prov_data				--> Object of ProvenanceData that contains
									pre-filled provenance data
		tmats_chanid_source		--> Prepared TMATS channel ID to source map
		tmats_chanid_type		--> Prepared TMATS channel ID to type map
		packet_type_label		--> See ch10_packet_type.h
		md_file_path			--> Output directory and file name for metadata

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
    bool RecordVideoDataF0Metadata(ManagedPath input_ch10_file_path,
                                   const ParserConfigParams& user_config,
								   const ProvenanceData& prov_data,
							       const std::map<std::string, std::string> tmats_chanid_source,
								   const std::map<std::string, std::string> tmats_chanid_type,
								   const std::string& packet_type_label,
								   const ManagedPath& md_file_path);

	

	/*
	Add provenance data to metadata output.

	Args:
		md						--> TIPMDDocument object to which metadata 
									will be added
		input_ch10_file_path	--> Path to ch10 file which was parsed
		packet_type_label		--> See ch10_packet_type.h
		prov_data				--> Object of ProvenanceData that contains
									pre-filled provenance data
							
	Return:
		True if no problems occur; false otherwise.
	*/
	bool RecordProvenanceData(TIPMDDocument& md, const ManagedPath& input_ch10_file_path,
		const std::string& packet_type_label, const ProvenanceData& prov_data);



	/*
	Add user configuration data to metadata output.

	Args:
		config_category			--> MDCategoryMap to which User configuration
									metadata will be added  
		user_config				--> ParserConfigParams object which has
									been configured with data from the
									user config file
	*/
	void RecordUserConfigData(std::shared_ptr<MDCategoryMap> config_category, 
		const ParserConfigParams& user_config);



    /*
	Record metadata specific to ARINC 429 format 0

	Args:
		input_ch10_file_path	--> Ch10 file that was parsed
		config					--> ParserConfigParams object which has
									been pre-configured with data from
									the parser_conf.yaml file
		prov_data				--> Object of ProvenanceData that contains
									pre-filled provenance data
		tmats_chanid_source		--> Prepared TMATS channel ID to source map
		tmats_chanid_type		--> Prepared TMATS channel ID to type map
		packet_type_label		--> See ch10_packet_type.h
		md_file_path			--> Output directory and file name for metadata

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
    bool RecordARINC429F0Metadata(ManagedPath input_ch10_file_path,
                                   const ParserConfigParams& user_config,
								   const ProvenanceData& prov_data,
							       const std::map<std::string, std::string> tmats_chanid_source,
								   const std::map<std::string, std::string> tmats_chanid_type,
								   const std::string& packet_type_label,
								   const ManagedPath& md_file_path);


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
                        std::vector<std::string>& tmats_vec);

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
	Convert the ch10_packet_type configuration map that is read from the
	parse_conf.yaml as a map<std::string, std::string> to a map<Ch10PacketType, bool>.

	Args:
		input_map	--> map<string, string> of the raw ch10_packet_type data
						structure found in the parse_conf.yaml file
		output_map	--> map<Ch10PacketType, bool> passed by reference

	Return:
		True if the conversion is successful, false otherwise.
	*/
    bool ConvertCh10PacketTypeMap(const std::map<std::string, std::string>& input_map,
                                  std::map<Ch10PacketType, bool>& output_map);

    /*
	Log the ch10_packet_type_map_. This is a convenience function to clean up
	the clutter that this code introduces.

	Args:
		pkt_type_config_map	--> Input map of Ch10PacketType to bool that
								represents the enable state of ch10 packet types
	*/
    void LogPacketTypeConfig(const std::map<Ch10PacketType, bool>& pkt_type_config_map);

    /*
	Create and verify output directories for enabled ch10 packet types.

	Args:
		output_dir			--> ManagedPath object giving the output directory into
								which packet type-specific dirs will be created
		base_file_name		--> ManagedPath object with the base file name on which
								to build the output directory name. May be a complete
								path to a file, in which case the parent path will be
								stripped and only the file name used, or a file name
								only.
		packet_enabled_map	--> Map of Ch10PacketType to boolean. True = enabled,
								False = disabled
		append_str_map		--> Map of Ch10PacketType to string. The mapped string is
								appended to the base_file_name prior and ought to
								include an extension if necessary, such as ".parquet"
								in the case of Parquet files. Map must contain at
								least all of the keys in the packet_enabled_map which
								are also mapped to true.
		pkt_type_output_dir_map --> Map of Ch10PacketType to specific output directory,
									the product of this function
		create_dir				--> True if the directory ought to be created, false
									otherwise

	Return:
		True if successful and all output directories were created; false otherwise.
	*/
    bool CreateCh10PacketOutputDirs(const ManagedPath& output_dir,
                                    const ManagedPath& base_file_name,
                                    const std::map<Ch10PacketType, bool>& packet_enabled_map,
                                    const std::map<Ch10PacketType, std::string>& append_str_map,
                                    std::map<Ch10PacketType, ManagedPath>& pkt_type_output_dir_map, bool create_dir);

    /*
	Generate a vector of maps of Ch10PacketType to ManagedPath. The path object
	is a file path to which data for the given Ch10PacketType
	ought to be written by the worker associated with the index of the vector
	from which the map was retrieved.

	Args:
		total_worker_count			--> Count of workers expected to be
										created to parse according to
										configuration settings
		pkt_type_output_dir_map		--> Map of Ch10PacketType to base output
										directory to which files associated
										with the packet type ought to be written.
										This is the pkt_type_output_dir_map from
										CreateCh10PacketOutputDirs.
		output_vec_mapped_paths		--> Vector of maps in which the index in
										the vector is the same as the worker which
										ought to utilize the mapped output file
										paths.
		file_extension				--> String not including the '.'.
										Ex: file_extension = 'txt'
	*/
    void CreateCh10PacketWorkerFileNames(const uint16_t& total_worker_count,
                                         const std::map<Ch10PacketType, ManagedPath>& pkt_type_output_dir_map,
                                         std::vector<std::map<Ch10PacketType, ManagedPath>>& output_vec_mapped_paths,
                                         std::string file_extension);

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
	Combine channel ID to LRU address maps from vector of maps,
	where each map in the vector corresponds to a map retrieved from
	a worker. The output map is used in another function where it is
	recorded to metadata.

	Args:
		output_chanid_lruaddr_map	--> Output map, ch10 channel ID to set of
										all LRU addresses observed on the bus
										identifed by the given channel ID
		chanid_lruaddr1_maps		--> Vector of maps of channel ID to set of
										LRU addresses associated with the channel
										ID for the TX/RX command word
		chanid_lruaddr2_maps		--> Vector of maps of channel ID to set of
										LRU addresses associated with the channel
										ID for the RX command word for RT to RT
										messages. Must have size() equal to
										chanid_lruaddr1_maps.

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
    bool CombineChannelIDToLRUAddressesMetadata(
        std::map<uint32_t, std::set<uint16_t>>& output_chanid_lruaddr_map,
        const std::vector<std::map<uint32_t, std::set<uint16_t>>>& chanid_lruaddr1_maps,
        const std::vector<std::map<uint32_t, std::set<uint16_t>>>& chanid_lruaddr2_maps);

    /*
	Combine channel ID to command words maps from a vector of maps,
	where each map in the vector corresponds to a map of observed channel ID
	to command words retrieved from a worker. The output map is used in
	another function where it is recorded to metadata.

	Args:
	output_chanid_commwords_map	--> Output map, ch10 channel ID to a vector
									of vectors. Each final vector must have size()
									equal to 2, where the first entry is TX/RX
									command word and second entry is the RX
									command word in the case of RT to RT type
									messages. So each channel ID is mapped to a
									vector of command words that were observed
									on the bus during parsing. The bus is not
									known at this time, only the channel ID
									which represents a specific bus.
	chanid_commwords_maps		--> Vector of maps of channel ID to set of
									uint32_t value which is: commword1 << 16
									+ commword2, where commword1 will be placed
									in the first position of the size-2 vector
									mentioned in the argument above, and
									commword2 will be placed in the second
									position.

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
    bool CombineChannelIDToCommandWordsMetadata(
        std::map<uint32_t, std::vector<std::vector<uint32_t>>>& output_chanid_commwords_map,
        const std::vector<std::map<uint32_t, std::set<uint32_t>>>& chanid_commwords_maps);

    /*
	Combine channel ID to 429 labels maps from a vector of maps,
	where each map in the vector corresponds to a map of observed channel ID
	to 429 labels retrieved from a worker. The output map is used in
	another function where it is recorded to metadata.

	Args:
	output_chanid_labels_map	--> Output map, ch10 channel ID to set of
									all 429 Labels observed on the bus
									identifed by the given channel ID.
	chanid_labels_maps  		--> Vector of maps of channel ID to set of ARINC
									429 labels associated with the channel ID.

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
    bool CombineChannelIDToLabelsMetadata(
        std::map<uint32_t, std::set<uint16_t>>& output_chanid_labels_map,
        const std::vector<std::map<uint32_t, std::set<uint16_t>>>& chanid_labels_maps);

    /*
	Combine channel ID to 429 IPDH Bus Numbers maps from a vector of maps,
	where each map in the vector corresponds to a map of observed channel ID
	to 429 IPDH Bus Numbers retrieved from a worker. The output map is used in
	another function where it is recorded to metadata.

	Args:
	output_chanid_busnumbers_map	--> Output map,ch10 channel ID to set of
									all IPDH Bus Numbers observed on the bus
									identifed by the given channel ID.
	chanid_busnumbers_maps		--> Vector of maps of channel ID to set of ARINC
									429 IPDH Bus Numbers associated with the channel
									ID.

	Return:
		True if no errors, false if errors occur and
		execution ought to stop.
	*/
    bool CombineChannelIDToBusNumbersMetadata(
        std::map<uint32_t, std::set<uint16_t>>&  output_chanid_busnumbers_map,
        const std::vector<std::map<uint32_t, std::set<uint16_t>>>& chanid_busnumbers_maps);

    /*
	Combine the channel ID to minimum timestamps map from each worker, already
	compiled as a vector maps, into the channel ID to absolute minimum timestamps
	map.

	output_chanid_to_mintimestamp_map	--> Output map, channel ID to minimum
											video packet timestamp for the channel
											ID. Timestamps in units of nanoseconds
											since the epoch (Unit time).
	chanid_mintimestamp_maps			--> Vector of maps of channel ID to minimum
											video packet time as observed by each
											worker.

	*/
    void CreateChannelIDToMinVideoTimestampsMetadata(
        std::map<uint16_t, uint64_t>& output_chanid_to_mintimestamp_map,
        const std::vector<std::map<uint16_t, uint64_t>>& chanid_mintimestamp_maps);

    /*
	Collect raw TMATS strings into a single string and write to disk.
	Use TMATSParser to parse and associate TMATS metadata into maps
	in preparation for recording to yaml metadata file.

	Args:
	tmats_vec					--> Vector of strings into which workers will push
									TMATS matter
	tmats_file_path				--> Complete path including the output file name
									to which TMATS matter is recorded
	TMATsChannelIDToSourceMap	--> Output artifact maps channel ID to source
	TMATsChannelIDToTypeMap		--> Output artifact maps channel ID to type

	*/
    void ProcessTMATS(const std::vector<std::string>& tmats_vec,
                      const ManagedPath& tmats_file_path,
                      std::map<std::string, std::string>& TMATsChannelIDToSourceMap,
                      std::map<std::string, std::string>& TMATsChannelIDToTypeMap);
};

#endif