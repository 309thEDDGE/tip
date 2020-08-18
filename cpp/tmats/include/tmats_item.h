#ifndef TMATSITEM_H
#define TMATSITEM_H

#include <string>
#include <cstdint>
#include <vector>
#include <regex>
#include <limits>

enum class TMATSDataType : uint8_t
{
	INT = 0,
	FLT = 1,
	STR = 2,
};


class TMATSItem
{
private:
	std::vector<uint16_t> ids;
	std::vector<std::vector<uint16_t>> indices;
	const std::string index_markers;
	const std::string nint_regex;
	std::string raw_code;
	std::string raw_regex;
	std::regex match_regex;
	std::vector<std::string> raw_data;
	TMATSDataType dtype;
	std::string item_type_desc();
	void create_regex();

public:
	TMATSItem(std::string code, TMATSDataType data_type);
	TMATSItem(const TMATSItem& c);
	void operator = (const TMATSItem& c);
	~TMATSItem();

	void data(uint16_t, int32_t&);
	void data(uint16_t, float&);
	void data(uint16_t, std::string&);
	std::string code();
	size_t count();
	std::vector<uint16_t> match_ids(uint16_t);
	void match_item(std::string test_code, std::string rawdata);
	std::vector<uint16_t> subids(uint16_t);
	
};

#endif 