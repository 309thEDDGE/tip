#ifndef BINARY_COMPARISON_MAIN_H_
#define BINARY_COMPARISON_MAIN_H_

#include <cstdio>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include "sysexits.h"
#include "cli_group.h"

int BinCompMain(int argc, char** argv);

bool OpenFile(const char* path, std::ifstream& ifs);

std::streamsize ReadBytes(std::ifstream& ifs, std::streamsize read_count, char* data);

bool ConfigureCLI(CLIGroup& cli_group, bool& help_requested, std::string& truth_path_str,
    std::string& test_path_str);



#endif  // BINARY_COMPARISON_MAIN_H_