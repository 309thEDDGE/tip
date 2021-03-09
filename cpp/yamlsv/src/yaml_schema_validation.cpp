#include "yaml_schema_validation.h"

YamlSV::~YamlSV()
{

}

YamlSV::YamlSV() : parse_text_()
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

	if (user_schema_node.size() == 0)
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

//bool YamlSV::MakeSchemaNode(YAML::Node& output_node, const YAML::Node& user_schema_node,
//	std::vector<LogItem>& log_output)
//{
//	// output_node should be initially empty
//	if (output_node.size() == 0)
//	{
//		AddLogItem(log_output, LogLevel::WARN, "output node is not empty");
//		return false;
//	}
//
//	// 
//	return true;
//}

//bool YamlSV::ProcessSchemaNode(YAML::Node& output_node, const YAML::Node& user_schema_node,
//	std::vector<LogItem>& log_output)
//{
//	std::string key;
//	std::string val;
//	uint8_t type_val;
//	bool res = false;
//	// Decide if current node level is a Scalar, Sequence, or Map.
//	if (user_schema_node.IsMap())
//	{
//		// Iterate over map.
//		for (YAML::const_iterator it = user_schema_node.begin(); it != user_schema_node.end(); ++it)
//		{
//			key = it->first.as<std::string>();
//			val = it->second.as<std::string>();
//
//			// Create the same node in the output_node.
//			//if (key == not_defined_str)
//			if (!GetTypeCode(val, type_val))
//				return false;
//			output_node[key] = type_val;
//		}
//	}
//	else if (user_schema_node.IsSequence())
//	{
//
//	}
//	else if (user_schema_node.IsScalar())
//	{
//
//	}
//	else
//	{
//
//	}
//	return true;
//}

//bool YamlSV::GetTypeCode(const std::string& type_str, uint8_t& type_val)
//{
//	if (string_to_schema_type_map.count(type_str) == 1)
//	{
//		type_val = string_to_schema_type_map.at(type_str);
//		return true;
//	}
//	return false;
//}

bool YamlSV::ProcessNode(const YAML::Node& test_node, const YAML::Node& schema_node,
	std::vector<LogItem>& log_output)
{
	std::string key;
	std::string val;
	// current node level is a Scalar, Sequence, or Map.
	if (schema_node.IsMap())
	{
		// Iterate over map.
		for (YAML::const_iterator it = schema_node.begin(); it != schema_node.end(); ++it)
		{
			key = it->first.as<std::string>();
			val = it->second.as<std::string>();
			if (!VerifyType(val, test_node[key].as<std::string>()))
				return false;
			// Create the same node in the output_node.
			//if (key == not_defined_str)

		}
	}
	else if (schema_node.IsSequence())
	{

	}
	else if (schema_node.IsScalar())
	{

	}
	else
	{

	}
	return true;
}

bool YamlSV::VerifyType(const std::string& str_type, const std::string& test_val)
{
	if (schema_string_type_set.count(str_type) == 0)
		return false;

	// No need to check string. Anything can be interpreted as a string.
	switch (string_to_schema_type_map.at(str_type))
	{
		case static_cast<uint8_t>(YamlSVSchemaType::INT) :
		{
			return parse_text_.TextIsInteger(test_val);
		}
		case static_cast<uint8_t>(YamlSVSchemaType::FLT) :
		{
			return parse_text_.TextIsFloat(test_val);
		}
	}
	return true;
}