#pragma once

#ifndef PARQUET_NOAAIGRA_H
#define PARQUET_NOAAIGRA_H

#include "parquet_context.h"
#include "Parser.h"
#include "Columnizer.h"
#include <random>

class IGRAData : public ParquetContext
{
private:
	const uint64_t normal_dist_count_;
	void create_schema();

public:
	IGRAData();
	void write_columns(Columnizer& c, std::string& station_name);
};


#endif