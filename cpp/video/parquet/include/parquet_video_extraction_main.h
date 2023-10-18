#ifndef PARQUET_VIDEO_EXTRACTION_MAIN_H_
#define PARQUET_VIDEO_EXTRACTION_MAIN_H_

#include <string>
#include <cstdio>
#include <ctime>
#include <cinttypes>
#include <chrono>
#include "sysexits.h"
#include "parquet_video_extraction.h"
#include "cli_group.h"

typedef std::chrono::high_resolution_clock Clock;

int PqVidExtractMain(int argc, char** argv);

bool ConfigurePqVidExtractCLI(CLIGroup& cli_group, bool& help_requested, std::string& input_path_str,
    std::string& output_path_str);

#endif  // PARQUET_VIDEO_EXTRACTION_MAIN_H_