#include "comparator.h"
#include <ctime>
#include <chrono>

typedef std::chrono::high_resolution_clock Clock;

int main(int argc, char* argv[]) {

	
	auto t1 = Clock::now();
	
	Comparator comp;

	if (argc < 3)
	{
		printf("Requires two parquet file paths as arguments");
		return 0;
	}

	bool valid_path = comp.Initialize(argv[1], argv[2]);

	if (!valid_path)
	{
		// Indicate a NULL comparison result. Ought to occur if the
		// the program is in error state, either file1 or file2 does
		// not exist, or a .parquet directory does not contain any 
		// parquet files. It looks like Comparator.Initialize()
		// (via OpenNextParquetFile(), called by SetPQPath())
		// returns false when either file1 or file2 does not contain
		// .parquet files.
		return 1;
	}

	comp.CompareAll();
	
	auto t2 = Clock::now();
	printf("\nElapsed Time: %d seconds\n", 
		std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count());

	return 0;
}