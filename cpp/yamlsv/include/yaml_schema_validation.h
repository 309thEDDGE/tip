#ifndef YAML_SCHEMA_VALIDATION_H_
#define YAML_SCHEMA_VALIDATION_H_

#include <vector>
#include <cstdarg>
#include "yaml-cpp/yaml.h"
#include "parse_text.h"
#include "yamlsv_log_item.h"
#include "yamlsv_schema.h"

class YamlSV
{
private:
	//YAML::Node schema_node_;
	ParseText parse_text_;

	static const int buff_size_ = 512;
	char buffer_[buff_size_];

public:
	YamlSV();
	~YamlSV();

	/*
	Validate a yaml node against a schema node.

	Args:
		test_node	--> root node of yaml matter under test
		schema_node	--> root node of yaml schema, see schema readme, above
		log_output	--> vector of LogItem filled during validation. Facilitates use
						of arbitrary logging mechanism.

	Return:
		True if yaml matter under test is validated, false otherwise.
	*/
	bool Validate(const YAML::Node& test_node, const YAML::Node& user_schema_node,
		std::vector<LogItem>& log_output);

	/////////////////////////////////////////////////////////////////////////////////
	// Internal use functions below
	/////////////////////////////////////////////////////////////////////////////////

	/*
	
	*/
	void AddLogItem(std::vector<LogItem>& log_output, LogLevel level, 
		std::string message);

	/*

	*/
	void AddLogItem(std::vector<LogItem>& log_output, LogLevel level,
		const char* fmt, ...);

	/*
	Simultaneously verify that the user-defined schema yaml complies with the standard
	defined here and create a new node with values that can be more efficiently 
	compared relative to the user schema types of "STR", "INT", etc.
	*/
	/*bool MakeSchemaNode(YAML::Node& output_node, const YAML::Node& user_schema_node,
		std::vector<LogItem>& log_output);*/

	/*bool ProcessSchemaNode(YAML::Node& output_node, const YAML::Node& user_schema_node,
		std::vector<LogItem>& log_output);*/

	//bool GetTypeCode(const std::string& type_str, uint8_t& type_val);

	bool ProcessNode(const YAML::Node& test_node, const YAML::Node& schema_node,
		std::vector<LogItem>& log_output);

	bool VerifyType(const std::string& str_type, const std::string& test_val);
};

#endif