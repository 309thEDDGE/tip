
#include <fstream>
#include <map>
#include <tuple>
#include <string>
#include <regex>
#include <iostream>
#include <cstdio>
#include <vector>
#include "iterable_tools.h"

class CodeName
{
   private:
    std::vector<std::string> subattrs;

   public:
    std::string plaintext;
    std::regex re;
    std::string regex_string;
    bool debug;
    CodeName(std::string, bool show_debug = false);
    std::map<std::string, int> groups(std::smatch);
};

class TMATSParser
{
   private:
    std::string raw;
    bool debug;

   public:
    TMATSParser(std::string, bool show_debug = false);
    std::map<std::string, std::string> MapAttrs(std::string, std::string);
};
