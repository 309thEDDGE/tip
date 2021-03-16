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
	Create a LogItem object with a given LogLevel and message.

	Args: 
		log_output	--> vector of LogItem into which the new LogItem
						will be added
		level		--> LogLevel specified for the new LogItem
		message		--> String message specified for the new LogItem
	*/
	void AddLogItem(std::vector<LogItem>& log_output, LogLevel level, 
		std::string message);

	/*
		Create a LogItem object with a given LogLevel and message,
		which is created by a format string and variable argument
		count.

	Args: 
		log_output	--> vector of LogItem into which the new LogItem
						will be added
		level		--> LogLevel specified for the new LogItem
		fmt			--> A printf style format C-string to be used 
						with the values passed in the remaining arguments
		...			--> N arguments passed to the format string
	*/
	void AddLogItem(std::vector<LogItem>& log_output, LogLevel level,
		const char* fmt, ...);

	/*
	Compare a schema node against a test node. Handles maps, sequences, and 
	scalars. Assumes the two nodes have the same structure. Intended to be 
	used recursively.

	Args:
		test_node	--> Node to be tested against the schema
		schema_node	--> Schema defined in YAML and read into a node
		log_output	--> Vector of log items to which will be added any 
						log entries generated during this function call

	Return:
		False if test matter does not conform to schema or an error occurs, 
		true otherwise.
	*/
	bool ProcessNode(const YAML::Node& test_node, const YAML::Node& schema_node,
		std::vector<LogItem>& log_output);

	/*
	Test a value passed as a string against one of the schema data types.
	Based on the ParseText::TextIsInteger/Float functions.

	Args:
		str_type	--> The data type retrieved from the schema to be used
						to verify the input value. One of the elements 
						in YamlSVSchemaType.
		test_val	--> String representation of the raw value to be tested
						for compatibility with the schema data type.

	Return:
		False if the str_type is invalid or the test_val is not compatible
		with the schema data type represented by str_type, true otherwise.
	*/
	bool VerifyType(const std::string& str_type, const std::string& test_val);

	/*
	Test a map element (key-value pair) against a schema element (key-value pair
	which contains the schema data type) via YAML::const_iterator (node iterators).

	Args:
		schema_it	--> Iterator to a schema map element
		test_it		--> Iterator to a test map element
		log_output	--> Vector of log items to which will be added any 
						log entries generated during this function call

	Return:
		False if the mapped test element is not compatible with the mapped
		schema data type or the mapped type is a map or sequence which
		has data not compatible with the schema data type indicated in the 
		schema-mapped sequence or map, otherwise true.
	*/
	bool TestMapElement(YAML::const_iterator& schema_it, 
		YAML::const_iterator& test_it, std::vector<LogItem>& log_output);

	/*
	Test a sequence node against a test node.

	Args:
		schema_node	--> Node containing a sequence with a single value, which
						is a string YamlSVSchemaType
		test_node	--> Node containing a sequence of n values, each of which
						is to be tested for compatibility with the schema data type
		log_output	--> Vector of log items to which will be added any 
						log entries generated during this function call

	Return:
		True if each value in the test_node is compatible with the schema 
		data type, false otherwise of if there is an error.
	*/
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