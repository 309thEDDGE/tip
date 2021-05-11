// parse_util.cpp

#include "parse_util.h"

double time_sec = 0;
uint64_t epoch_sec = 0;
uint64_t epoch_ns = 0;
time_t epoch_sec_time_t = 0;
uint64_t sec_frac_ns = 0;

void recover_4byte_date(uint64_t& time_stamp, std::tm*& timeinfo, uint32_t*& sec_fraction)
{
	// Get time since the epoch in seconds.
	epoch_sec = time_stamp / 1e9;
	epoch_ns = epoch_sec * 1e9;
	epoch_sec_time_t = time_t(epoch_sec);
	sec_frac_ns = time_stamp - epoch_ns;

	timeinfo = gmtime(&epoch_sec_time_t);
	//printf("year %u\n", timeinfo->tm_year);

	// Calculate the quantity of nanoseconds in the fraction.
	// Multiply by 10^7 for printing 7 fraction digits like DRA hexdump.
	*sec_fraction = uint32_t(sec_frac_ns / 100);
	
}

void recover_3byte_date(uint64_t& time_stamp, std::tm*& timeinfo, uint32_t*& sec_fraction)
{
	// Note: currently same function as 4byte date. Only difference is that the 
	// tm.year field will be 1970 because year is not encapsulated in the original 
	// tm struct from which the time_stamp integer was created.

	// Get time since the epoch in seconds.
	epoch_sec = time_stamp / 1e9;
	epoch_ns = epoch_sec * 1e9;
	epoch_sec_time_t = time_t(epoch_sec);
	sec_frac_ns = time_stamp - epoch_ns;

	timeinfo = gmtime(&epoch_sec_time_t);
	//printf("year %u\n", timeinfo->tm_year);

	// Calculate the quantity of nanoseconds in the fraction.
	// Multiply by 10^7 for printing 7 fraction digits like DRA hexdump.
	*sec_fraction = uint32_t(sec_frac_ns / 100);
}