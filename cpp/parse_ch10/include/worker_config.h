
#ifndef WORKER_CONFIG_H_
#define WORKER_CONFIG_H_

#include <cstdint>
#include <map>
#include <memory>
#include <sstream>
#include "managed_path.h"
#include "parser_paths.h"
#include "binbuff.h"
#include "ch10_packet_type.h"
#include "spdlog/spdlog.h"

class WorkerConfig
{
   public:
    // The 0-indexed ParseWorker index to which this configuration
    // is associated.
    uint16_t worker_index_;

    ///////////////////////////// new ////////////////////////////////

    // Count of all workers, stored for logical checks on worker_index_.
    uint16_t total_worker_count_;

    // Configured data ingest size, append mode ingest size, and total
    // ch10 file size.
    uint64_t read_bytes_;
    uint64_t actual_read_bytes_;
    uint64_t append_read_bytes_;
    uint64_t total_bytes_;

    std::ifstream* input_stream_;

    /////////////////////////////////////// end new //////////////////////////////

    // Buffer which holds ch10 binary data to be parsed
    std::unique_ptr<BinBuff> bb_unique_;
    BinBuff * bb_;

    // Absolute position in ch10 at which ParseWorker begins parsing
    uint64_t start_position_;

    // Absolute position in ch10 at which ParseWorker ends parsing
    uint64_t last_position_;

    // True if worker is the final worker used to parse a Ch10
    // This variable is used to indicate that the file writers
    // need to be closed.
    bool final_worker_;

    // True if the worker is an append_mode worker, which
    // parsers all the packets starting at the last position
    // until the first time data packet (TDP).
    bool append_mode_;

    // Output paths for each Ch10 packet type, specific to the
    // ParseWorker for which the map is created.
    std::map<Ch10PacketType, ManagedPath> output_file_paths_;

    // Ch10 packet type configuration
    std::map<Ch10PacketType, bool> ch10_packet_type_map_;

    WorkerConfig();

    /*
    Check the configuration of a pre-configured WorkerConfig instance.
    This is the handler for append mode vs non append mode.

    Args:
        worker_config       --> WorkerConfig instance

    Return:
        False if configuration is invalid; true otherwise.
    */
	virtual bool CheckConfiguration();



    /*
    Set member variables with initial configuration.

    Args:
		worker_count		--> Total count of workers, also equal to .size() of
								worker_vecs_, threads_vec_, and worker_config_vec_
		worker_index		--> Index of the objects in worker_vecs_, threads_vec_,
								and worker_config_vec_ which are being configured
								for parsing
		read_size			--> Size of chunk in bytes of ch10 to parse for each
								first-pass worker
        append_read_size    --> Size of chunk in bytes to read for appending
                                bytes which were cut off in arbitrary chunking
		total_size			--> Total size of the ch10 in bytes
		ch10_input_stream	--> Initialized input stream for the ch10 file to
								be parsed
        parser_paths        --> Configure ParserPaths object

    Return:
        True if all variables can be assigned from inputs; false otherwise. 
    */
    bool Initialize(const uint16_t& worker_count, const uint16_t& worker_index, 
        const uint64_t& read_size, const uint32_t& append_read_size, const uint64_t& total_size, 
        std::ifstream& ch10_input_stream, const ParserPaths* parser_paths);



    //////////////////////////////////////////////////////////////////////////////
    // Functions below are considered to be internal functions. They
    // are made public to facilitate testing.
    //////////////////////////////////////////////////////////////////////////////

    /*
    Check the non append mode configuration.
    
    Args:
        worker_config       --> WorkerConfig instance

    Return:
        False if configuration is invalid; true otherwise.
    */
    bool CheckNonAppendModeConfig();



    /*
    Check the append mode configuration.
    
    Args:
        worker_config       --> WorkerConfig instance

    Return:
        False if configuration is invalid; true otherwise.
    */
    bool CheckAppendModeConfig();

};

#endif