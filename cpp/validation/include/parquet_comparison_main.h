#ifndef PARQUET_COMPARISON_MAIN_H_
#define PARQUET_COMPARISON_MAIN_H_

#include <ctime>
#include <chrono>
#include <cinttypes>
#include "sysexits.h"
#include "comparator.h"
#include "cli_group.h"


typedef std::chrono::high_resolution_clock Clock;

int PqCompMain(int argc, char** argv);

bool ConfigurePqCompCLI(CLIGroup& cli_group, bool& help_requested, std::string& truth_path_str,
    std::string& test_path_str);


#endif  // PARQUET_COMPARISON_MAIN_H_