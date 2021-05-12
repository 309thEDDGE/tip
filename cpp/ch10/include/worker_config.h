
#ifndef WORKER_CONFIG_H_
#define WORKER_CONFIG_H_

#include <cstdint>
#include "managed_path.h"
#include "binbuff.h"
#include "ch10_packet_type.h"

class WorkerConfig
{
public:

	// The 0-indexed ParseWorker index to which this configuration
	// is associated.
	uint16_t worker_index_;

	// Buffer which holds ch10 binary data to be parsed
	BinBuff bb_;

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

	WorkerConfig() : worker_index_(0), start_position_(0),
		last_position_(0), final_worker_(false), append_mode_(false), bb_()
	{}
};

#endif