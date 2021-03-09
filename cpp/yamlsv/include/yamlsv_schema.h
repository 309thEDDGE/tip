#ifndef YAMLSV_SCHEMA_H_
#define YAMLSV_SCHEMA_H_

#include <cstdint>
#include <unordered_map>

enum class YamlSVSchemaType : uint8_t
{
	STR,
	INT,
	FLT,
	BOOL
};

const std::unordered_map<std::string, YamlSVSchemaType> string_to_schema_type_map = {
	{"STR", YamlSVSchemaType::STR},
	{"INT", YamlSVSchemaType::INT},
	{"FLT", YamlSVSchemaType::FLT},
	{"BOOL", YamlSVSchemaType::BOOL}
};

const std::string not_defined_str = "_NOT_DEFINED_";


#endif