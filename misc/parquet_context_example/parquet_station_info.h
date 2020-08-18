#pragma once
#include "parquet_context.h"
#include "Columnizer.h"
#include "Station_Columnizer.h"

class Parquet_Station_Info : public ParquetContext
{
private:
	void create_schema();

public:
	Parquet_Station_Info() {};
	void write_columns(Station_Columnizer& c);
};

