/*
//#include "arrow_test.h"
#include "parquet_noaa_igra.h"

int main(int argc, char* argv[])
{
	std::string path("C:\\Users\\A10\\Documents\\apache_arrow\\test_output\\out.parquet");
	//ArrowTest a(path);
	printf("after creating ArrowTest object\n");
	const uint64_t count = 5;
	int64_t append_count = count;
	const int64_t cost_count = 3;
	int64_t cost_count_per_row = cost_count;
	std::vector<uint64_t> time1 = { 34567544543, 34567544543, 345675466545, 345675466545, 345675466599 };
	std::vector<int32_t> id1 = { 0, 1, 2, 3, 4 };
	std::vector<uint16_t> cost1 = { 11, 22, 33, 44, 55, 66, 77, 88, 99, 111, 222, 333, 444, 555, 666 };
	//a.append(append_count, time1, id1, cost1, cost_count_per_row);
	

	std::vector < uint64_t > time2 = { 34568544543, 34568544543, 345685466545, 345685466545, 345685466599 };
	std::vector < int32_t > id2 = { 5, 6, 7, 8, 9 };
	std::vector < uint16_t > cost2 = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 110, 220, 330, 440, 550, 660 };
	//a.append(append_count, time2, id2, cost2, cost_count_per_row);

	IGRAData id;
	uint8_t ret_val = 0;
	/*ret_val = id.open_for_write(path, true);
	if (ret_val != 0)
	{	
		printf("bad return val on open_for_write()\n");
		return 0;
	}
	ret_val = id.append_to_column<uint64_t, arrow::UInt64Type>(time1, "time");
	if (ret_val != 0)
		printf("failure on append time\n");
	ret_val = id.append_to_column<int32_t, arrow::Int32Type>(id1, "ID");
	if (ret_val != 0)
		printf("failure on append ID\n");
	ret_val = id.append_lists_to_column<uint16_t, arrow::UInt16Type>(cost1, "cost");
	if (ret_val != 0)
		printf("failure on append cost\n");

	ret_val = id.append_to_column<int32_t, arrow::Int32Type>(id2, "ID");
	if (ret_val != 0)
		printf("failure on append ID\n");
	ret_val = id.append_lists_to_column<uint16_t, arrow::UInt16Type>(cost2, "cost");
	if (ret_val != 0)
		printf("failure on append cost\n");
	ret_val = id.append_to_column<uint64_t, arrow::UInt64Type>(time2, "time");
	if (ret_val != 0)
		printf("failure on append time\n");

	return 0;
}

*/