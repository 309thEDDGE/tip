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
	ParseText parse_text_;

	static const int buff_size_ = 512;
	char buffer_[buff_size_];

	std::string bool_tolower_;
	bool sequence_is_opt_;
	std::string sequence_str_type_;

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

	bool ProcessNode(const YAML::Node& test_node, const YAML::Node& schema_node,
		std::vector<LogItem>& log_output);

	bool VerifyType(const std::string& str_type, const std::string& test_val);

	bool TestMapElement(YAML::const_iterator& schema_it, 
		YAML::const_iterator& test_it, std::vector<LogItem>& log_output);

	bool TestSequence(const YAML::Node& schema_node, const YAML::Node& test_node, 
		std::vector<LogItem>& log_output);

	/*
	Get a string representing the standard schema type, after checking for 
	any modifier characters.

	Args:
		test_type	--> String representing the schema type as read from the 
						schema yaml.
		str_type	--> The string representation of the schema type if the 
						test_type is valid.
		is_opt		--> Set the value to true if the schema is valid

	Return:
		True if the test_type includes a valid schema type with or without
		valid modifier characters.
	*/
	bool CheckSequenceType(const std::string& test_type, std::string& str_type,
		bool& is_opt);

};

#endif