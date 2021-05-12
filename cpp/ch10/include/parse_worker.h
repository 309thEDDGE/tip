#ifndef PARSEWORKER_H
#define PARSEWORKER_H

/*
Execute ch10 parsing on a chunk of binary data using
parser_rewrite lib.
*/

#include <string>
#include <cstdio>
#include <set>
#include <atomic>
#include "ch10_packet_type.h"
#include "ch10_context.h"
#include "ch10_packet.h"
#include "binbuff.h"
#include "iterable_tools.h"
#include "managed_path.h"
#include "worker_config.h"
#include "spdlog/spdlog.h"

class ParseWorker
{
private:

	// True if worker has completed parsing, false otherwise
	std::atomic<bool> complete_;

	// Track Ch10 state and manipulate metadata
	Ch10Context ctx_;

	
public:

	// Used to obtain public data from this worker's Ch10Context instance
	const Ch10Context& ch10_context_;

	ParseWorker();

	/*
	Worker job completion status.

	Return:
		True if the worker has completed parsing or an error has occurred
		that caused parsing to cease early, false otherwise.
	*/
	std::atomic<bool>& CompletionStatus();

	/*
	The primary function of ParseWorker. To be called by passing an instance
	of ParseWorker as the first argument to the std::thread constructor, in which
	this operator() function is automatically executed. 

	Parses the Ch10 binary data loaded into the BinBuff object encapsulated in
	the WorkerConfig object. Uses other encapsulated data to initialize the Ch10Context
	owned by ParseWorker instance. Any matter from TMATs packets found by the worker
	are appended to the tmats_body_vec.

	Args:
		worker_config	--> WorkerConfig object defining the worker configuration
							state and binary data to be parsed
		tmats_body_vec	--> Vector to which any TMATs matter found in the binary 
							data are appended
	*/
	void operator()(WorkerConfig& worker_config, std::vector<std::string>& tmats_body_vec);
};

#endif 