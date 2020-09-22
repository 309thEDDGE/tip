
#include <fstream>
#include <map>
#include <string>
#include <regex>
#include <iostream>


class CodeName
{
private:
    std::vector<std::string> subattrs;
public:
    std::string plaintext;
    std::regex re;
    std::string regex_string;
    CodeName(std::string);
    std::map<std::string, int> groups(std::smatch);
};


class TMATSParser
{
private:
    std::string raw;
public:
    TMATSParser(std::string);
    std::map<std::string, std::string> MapAttrs(std::string, std::string);
};
