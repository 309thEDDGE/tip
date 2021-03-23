#ifndef YAMLSV_SCHEMA_H_
#define YAMLSV_SCHEMA_H_

#include <cstdint>
#include <unordered_map>
#include <set>

enum class YamlSVSchemaType : uint8_t
{
	STR = 0,
	INT = 1,
	FLT = 2,
	BOOL = 3,
};

const std::unordered_map<std::string, uint8_t> string_to_schema_type_map = {
	{"STR", static_cast<uint8_t>(YamlSVSchemaType::STR)},
	{"INT", static_cast<uint8_t>(YamlSVSchemaType::INT)},
	{"FLT", static_cast<uint8_t>(YamlSVSchemaType::FLT)},
	{"BOOL", static_cast<uint8_t>(YamlSVSchemaType::BOOL)}
};

const std::set<std::string> schema_string_type_set = { "STR", "INT", "FLT", "BOOL" };

namespace YamlSVSchemaTag
{
	const std::string not_defined_str = "_NOT_DEFINED_";
	const std::string not_defined_opt_str = "_NOT_DEFINED_OPT_";
}


#endif