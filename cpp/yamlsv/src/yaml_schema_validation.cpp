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

void YamlSV::AddLogItem(std::vector<LogItem>& log_output, LogLevel level,
	const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer_, buff_size_, fmt, args);
	std::string msg(buffer_);

	LogItem item(level, msg);
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
	std::string type_str;
	YAML::const_iterator schema_next;
	YAML::const_iterator test_current;
	//YAML::const_iterator test_next;
	// current node level is a Scalar, Sequence, or Map.
	if (schema_node.IsMap())
	{
		// TODO: Check if the current test_node is also a map and return false if not.

		// Iterate over map.
		test_current = test_node.begin();
		for (YAML::const_iterator it = schema_node.begin(); it != schema_node.end(); ++it)
		{
			key = it->first.as<std::string>();

			// If the key is the special tag "_NOT_DEFINED_", do not require a
			// key and rely on the position of the iterators.
			if (key == not_defined_str)
			{
				// While the current test key is not equal to the next schema key,
				// assume the items are duplicates with the schema defined by the current
				// value mapped to the not defined key.
				if (it == schema_node.end())
				{
					if (!TestOrProcess(it, test_current, log_output))
						return false;
				}
				else
				{
					schema_next = std::next(it, 1);
					while (test_current->first.as<std::string>() !=
						schema_next->first.as<std::string>())
					{
						printf("compared schema %s with test %s\n",
							schema_next->first.as<std::string>().c_str(),
							test_current->first.as<std::string>().c_str());

						if (!TestOrProcess(it, test_current, log_output))
							return false;

						test_current++;
					}
				}
				continue;
			}

			// If the key is not present in the test map, then do not proceed.
			if (!test_node[key])
			{
				AddLogItem(log_output, LogLevel::INFO, 
					"YamlSV::ProcessNode: Key %s in schema not present yaml",
					key.c_str());
				return false;
			}

			// If the mapped type is a scalar, check the value against the schema.
			if (!TestOrProcess(it, test_current, log_output))
				return false;

			/*if (test_current == test_node.end())
			{
				if (it != schema_node.end())
				{
					AddLogItem(log_output, LogLevel::INFO,
						"YamlSV::ProcessNode: test node iterator reached the end prematurely");
					return false;
				}
			}*/
			test_current++;
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
		case static_cast<uint8_t>(YamlSVSchemaType::BOOL) :
		{
			if (!(test_val == "True" || test_val == "False"))
				return false;
		}
	}
	return true;
}

bool YamlSV::TestOrProcess(YAML::const_iterator& schema_it, YAML::const_iterator& test_it,
	std::vector<LogItem>& log_output)
{
	// If the mapped type is a scalar, check the value against the schema.
	if (schema_it->second.IsScalar())
	{
		if (!test_it->second.IsScalar())
		{
			AddLogItem(log_output, LogLevel::INFO,
				"YamlSV::TestOrProcess: Value for key %s is not a scalar as indicated by the type %s",
				test_it->first.as<std::string>().c_str(),
				schema_it->second.as<std::string>().c_str());
			return false;
		}

		if (!VerifyType(schema_it->second.as<std::string>(),
			test_it->second.as<std::string>()))
		{
			AddLogItem(log_output, LogLevel::INFO,
				"YamlSV::TestOrProcess: Value for key %s does not match type %s",
				test_it->first.as<std::string>().c_str(), 
				schema_it->second.as<std::string>().c_str());
			return false;
		}
	}

	// If the mapped type is another map or a sequence, process the lower node
	// separately.
	else
	{
		if (!ProcessNode(test_it->second, schema_it->second, log_output))
			return false;
	}
	return true;
}