#ifndef PARSE_TEXT_H
#define PARSE_TEXT_H

#include <string>
#include <vector>
#include <map>
#include "iterable_tools.h"

class ParseText
{

private:


public:
	ParseText() {};
	~ParseText() {};

	std::vector<std::string> Split(std::string input_string, const char& delim);
	bool ConvertInt(const std::string& convert_string, int& output);
	bool ConvertDouble(const std::string& convert_string, double& output);
	bool CheckForExisitingChar(const std::string& input_string, char character);
	std::string RemoveTrailingChar(const std::string& input_string, char character);
	bool TextIsInteger(const std::string& input_string);
	bool TextIsFloat(const std::string& input_string);

	// Extract quoted (in quotation marks) and non-quoted text into
	// maps with the key indicating the order in which the section originally
	// occurred. Return true if quoted sections present and false otherwise.
	// Input maps are only filled if true is returned.
	//
	// Strings returned in the quoted sections map have the original quotes
	// removed.
	bool ExtractQuotedSections(const std::string& input_string, 
		std::map<int, std::string>& quoted_sections,
		std::map<int, std::string>& unquoted_sections);
};

#endif