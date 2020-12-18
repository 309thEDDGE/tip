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

	std::string path1(argv[1]);
	std::string path2(argv[2]);
	comp.Initialize(ManagedPath(path1), ManagedPath(path2));
	comp.CompareAll();
	
	auto t2 = Clock::now();
	printf("\nElapsed Time: %d seconds\n", 
		std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count());

	return 0;
}