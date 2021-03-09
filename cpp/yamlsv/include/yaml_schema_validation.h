#ifndef YAML_SCHEMA_VALIDATION_H_
#define YAML_SCHEMA_VALIDATION_H_

#include <vector>
#include "yaml-cpp/yaml.h"
#include "yamlsv_log_item.h"

class YamlSV
{
private:
	YAML::Node schema_node_;

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
	bool MakeSchemaNode(YAML::Node& output_node, const YAML::Node& user_schema_node,
		std::vector<LogItem>& log_output);
};

#endif