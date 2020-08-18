#include "parquet_video_extraction.h"
#include <ctime>
#include <chrono>

typedef std::chrono::high_resolution_clock Clock;

int main(int argc, char* argv[]) {

	auto t1 = Clock::now();
	
	ParquetVideoExtraction pe;

	bool valid_path = pe.Initialize(argv[1]);

	if (!valid_path)
	{
		return 0;
	}

	pe.ExtractTS();
	
	auto t2 = Clock::now();
	printf("\nElapsed Time: %d seconds\n", std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count());

	return 0;
}