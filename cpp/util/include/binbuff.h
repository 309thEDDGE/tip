#ifndef BINBUFF_H
#define BINBUFF_H

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <vector>

/*
Spdlog is used here and there is a conflict with a type, namespace, 
or object defined in spdlog that is later interpreted incorrectly when 
loading the parquet/arrow headers. 

There is something in the build system that utilizes BinBuff prior to 
parquet and thus tries to build binbuff first in the util lib and the
definition is set prior to including parquet/arrow for other objects
defined in the util lib. This is the reason for parquet/arrow inclusion
here, in BinBuff where it's not needed. 

I think a better solution is to break off all parquet/arrow related classes
into a separate library, call it parquet_util, in which case the parquet/arrow
headers will be included first in each class. 
*/
//#include <arrow/api.h>
//#include <arrow/io/api.h>
//#include <parquet/arrow/reader.h>
//#include <parquet/arrow/schema.h>
//#include <parquet/arrow/writer.h>
#include "spdlog/spdlog.h"

class BinBuff
{
	private:

		// Count of bytes requested to be read from the ifstream.
		// Set in Initialize.
		uint64_t read_count_;

		// Vector of binary data read in when Initialize is called, 
		// if successful. The data payload and and purpose of this
		// this class.
		std::vector<uint8_t> bytes_;

		// The read position within the data payload. Zero-indexed
		// in [0, <count of bytes in buffer> - 1].
		uint64_t pos_;

		// The total file size of the file to which the ifstream passed
		// to Initialize is connected. Used to check for overread.
		uint64_t file_size_;

		// The position within the file to which ifstream is connected
		// that is read from to fill the bytes_ buffer.
		uint64_t source_pos_;

		// If true, indicates that bytes_ has been filled correctly 
		// with either the requested count or all the byte remaining
		// in the file pointed to by the ifstream passed to Initialize.
		bool is_initialized_;
	
	public:
		BinBuff();
		virtual ~BinBuff();

		// Convenient access to the current position within the buffer.
		const uint64_t& position_;

		// Read binary 'read_count' bytes at position 'read_pos' from 'infile' into 
		// buffer. 'file_size' is the byte count of the input file connected to 
		// 'infile'. Returns the count of bytes actually read into the buffer.
		// If the requested read count is greater than source file size or the 
		// count of bytes from the read position within the source file to the 
		// end of the file is less than the requested read count, fewer bytes
		// will be read into the buffer than requested and the count of bytes actually
		// read will be returned.
		uint64_t Initialize(std::ifstream& infile, 
			const uint64_t& file_size, const uint64_t& read_pos, const uint64_t& read_count);

		// Find and return the location within the buffer of the first
		// occurrence of 'pattern'. Regardless of pattern data type, 
		// check for existence of pattern beginning at every byte,
		// starting at the current position within the buffer, increaing 
		// incrementally until the check position reaches the end of the
		// buffer minus the size of the pattern.
		template<typename T>
		uint64_t FindPattern(const T& pattern) const;
		template<typename T>
		uint64_t FindPattern(const T& pattern, const uint64_t& start_search_pos) const;

		// Find and return the position within the buffer of all occurrences 
		// of the given pattern.
		template<typename T>
		std::vector<uint64_t> FindAllPattern(T& pattern) const;

		// Get a pointer to the raw buffer data at the current
		// read position. 
		virtual const uint8_t* Data() const; 

		// Advance the read position relative to the current position.
		// The beginning of a new buffer is initialized at position
		// zero. BinBuff::AdvanceReadPos(10) set the position 
		// to 10. A second call with value ten sets the position
		// to 20.
		// 
		// Return 0 if read position was moved by the requested
		// count and 1 if the move failed.
		virtual uint8_t AdvanceReadPos(const uint64_t& count);

		// Set the absolute read position without regard
		// to the current position of the buffer.
		//
		// Return 0 if the position was successfully set at
		// the requested position or 1 if unsuccessful.
		uint8_t SetReadPos(const uint64_t& position);

		// Return true if the count of bytes can be 
		// read from the buffer at the current position 
		// without reading beyond the buffer size.
		// The count of bytes available includes the
		// byte at the current position.
		virtual bool BytesAvailable(const uint64_t& count) const;

		// Return true if Initialize() has been called.
		bool IsInitialized() const;

		// Get the count of bytes actually read into the buffer
		// after initialization. This is the functional size
		// of the buffer.
		uint64_t Size() const;
	
};

template<typename T>
uint64_t BinBuff::FindPattern(const T& pattern) const
{
	const T* val;
	size_t pattern_size = sizeof(pattern);
	for (uint64_t curr_pos = pos_; curr_pos < (read_count_ - pattern_size + 1); curr_pos++)
	{
		val = (const T*)(bytes_.data() + curr_pos);
		if (*val == pattern)
		{
			return curr_pos;
		}
	}
	return UINT64_MAX;
}

template<typename T>
uint64_t BinBuff::FindPattern(const T& pattern, const uint64_t& start_search_pos) const
{
	const T* val;
	size_t pattern_size = sizeof(pattern);
	for (uint64_t curr_pos = start_search_pos; curr_pos < (read_count_ - pattern_size + 1); curr_pos++)
	{
		val = (const T*)(bytes_.data() + curr_pos);
		if (*val == pattern)
		{
			return curr_pos;
		}
	}
	return UINT64_MAX;
}

template<typename T>
std::vector<uint64_t> BinBuff::FindAllPattern(T& pattern) const
{
	uint64_t pattern_size = sizeof(pattern);
	uint64_t found_at = UINT64_MAX;
	std::vector<uint64_t> locs;
	uint64_t init_pos = pos_;
	while (init_pos < read_count_)
	{
		found_at = FindPattern(pattern, init_pos);
		if (found_at == UINT64_MAX)
			break;
		locs.push_back(found_at);

		// Move position past the found_at position plus
		// the size of the search pattern. It is meaningless
		// to search for the pattern within the trailing bytes
		// of the pattern which was just found. Move to the first
		// bytes beyond the pattern and continue the search.
		init_pos = found_at + pattern_size;
	}

	return locs;
}

#endif