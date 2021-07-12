#include "parquet_video_extraction.h"
#include <ctime>
#include <chrono>

typedef std::chrono::high_resolution_clock Clock;

int main(int argc, char* argv[])
{
    auto t1 = Clock::now();

    ParquetVideoExtraction pe;

    std::string input_pq_video_dir = argv[1];
    bool valid_path = pe.Initialize(ManagedPath(input_pq_video_dir));

    if (!valid_path)
    {
        return 0;
    }

    pe.ExtractTS();

    auto t2 = Clock::now();
    printf("\nElapsed Time: %d seconds\n", std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count());

    return 0;
}