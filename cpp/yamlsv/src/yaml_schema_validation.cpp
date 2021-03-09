#include "yaml_schema_validation.h"

YamlSV::~YamlSV()
{

}

YamlSV::YamlSV()
{

}

bool YamlSV::Validate(const YAML::Node& test_node, const YAML::Node& user_schema_node,
	std::vector<LogItem>& log_output)
{
	bool is_validated = true;

	// Return false if nodes are empty.
	if (test_node.size() == 0)
	{
		is_validated = false;
		AddLogItem(log_output, LogLevel::WARN, "test node is empty");
	}

	if (schema_node.size() == 0)
	{
		is_validated = false;
		AddLogItem(log_output, LogLevel::WARN, "schema node is empty");
	}

	if (!is_validated)
		return false;
	return true;
}

void YamlSV::AddLogItem(std::vector<LogItem>& log_output, LogLevel level,
	std::string message)
{
	LogItem item(level, message);
	log_output.push_back(item);
}