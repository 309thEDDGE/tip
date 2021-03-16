#include "yaml_schema_validation.h"

YamlSV::~YamlSV()
{

}

YamlSV::YamlSV() : parse_text_(), is_opt_(false)
{

}

bool YamlSV::Validate(const YAML::Node& test_node, const YAML::Node& user_schema_node,
	std::vector<LogItem>& log_output)
{
	// Return false if nodes are empty.
	if (test_node.size() == 0)
	{
		AddLogItem(log_output, LogLevel::WARN, "test node is empty");
		return false;
	}

	if (user_schema_node.size() == 0)
	{
		AddLogItem(log_output, LogLevel::WARN, "schema node is empty");
		return false;
	}

	return ProcessNode(test_node, user_schema_node, log_output);
}

void YamlSV::AddLogItem(std::vector<LogItem>& log_output, LogLevel level,
	std::string message)
{
	LogItem item(level, message);
	//item.Print();
	log_output.push_back(item);
}

void YamlSV::AddLogItem(std::vector<LogItem>& log_output, LogLevel level,
	const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer_, buff_size_, fmt, args);
	std::string msg(buffer_);

	AddLogItem(log_output, level, msg);
}

bool YamlSV::ProcessNode(const YAML::Node& test_node, const YAML::Node& schema_node,
	std::vector<LogItem>& log_output)
{
	std::string key;
	std::string type_str;
	YAML::const_iterator schema_next;
	YAML::const_iterator test_current;
	
	// current node level is a Scalar, Sequence, or Map.
	if (schema_node.IsMap())
	{
		// Iterate over map.
		test_current = test_node.begin();
		for (YAML::const_iterator it = schema_node.begin(); it != schema_node.end(); ++it)
		{
			key = it->first.as<std::string>();
			if (test_current == test_node.end())
			{
				// Handle case in which the _NOT_DEFINED_OPT_ is the key, there is only
				// one entry in the schema and zero entries in the test map.
				if (key == YamlSVSchemaTag::not_defined_opt_str && 
					schema_node.size() == 1 &&
					test_node.size() == 0)
				{
					AddLogItem(log_output, LogLevel::DDEBUG,
						"YamlSV::ProcessNode: Schema node size 1 and test node empty");
					return true;
				}
				AddLogItem(log_output, LogLevel::INFO,
					"YamlSV::ProcessNode: Reached end of test node prior to end of schema node");
				return false;
			}

			AddLogItem(log_output, LogLevel::DDEBUG,
				"YamlSV::ProcessNode: Schema map key = \"%s\"", key.c_str());

			// If the key is the special tag "_NOT_DEFINED_", do not require a
			// key and rely on the position of the iterators.
			if (key == YamlSVSchemaTag::not_defined_str || key == YamlSVSchemaTag::not_defined_opt_str)
			{
				AddLogItem(log_output, LogLevel::DDEBUG,
					"YamlSV::ProcessNode: Schema map key is special tag: \"%s\"", key.c_str());

				// While the current test key is not equal to the next schema key,
				// assume the items are duplicates with the schema defined by the current
				// value mapped to the not defined key.
				schema_next = std::next(it, 1);
				if (schema_next == schema_node.end())
				{
					AddLogItem(log_output, LogLevel::DDEBUG,
						"YamlSV::ProcessNode: Current schema entry is last in node");
					while (test_current != test_node.end())
					{

						if (!TestMapElement(it, test_current, log_output))
							return false;

						test_current++;
					}
				}
				else
				{
					while (test_current->first.as<std::string>() !=
						schema_next->first.as<std::string>())
					{
						AddLogItem(log_output, LogLevel::DDEBUG,
							"YamlSV::ProcessNode: test key \"%s\""
							" does not equal next non-wildcard schema key \"%s\"", 
							test_current->first.as<std::string>().c_str(),
							schema_next->first.as<std::string>().c_str());

						if (!TestMapElement(it, test_current, log_output))
							return false;

						test_current++;
					}
				}
				continue;
			}

			// If the key is not present in the test map, then do not proceed.
			if (test_current->first.as<std::string>() != key)
			{
				AddLogItem(log_output, LogLevel::INFO, 
					"YamlSV::ProcessNode: Key %s in schema not present in yaml",
					key.c_str());
				return false;
			}

			// If the mapped type is a scalar, check the value against the schema.
			if (!TestMapElement(it, test_current, log_output))
				return false;

			test_current++;
		}
	}
	else if (schema_node.IsSequence())
	{
		test_current = test_node.begin();

		// If the size of the schema sequence is one, then there is no
		// need to iterate.
		if (schema_node.size() == 1)
		{

			// Process the deeper level if the entry is also a 
			// sequence. 
			if (schema_node[0].IsSequence())
			{
				// Then the first element in the test node must also be a sequence.
				/*if (!test_node[0].isSequence())
				{

				}*/
				if (!ProcessNode(test_node[0], schema_node[0], log_output))
					return false;
			}
			else
			{
				if (!TestSequence(schema_node, test_node, log_output))
					return false;
			}
		}
		else
		{
			// Iterate over the schema sequence.
			for (YAML::const_iterator it = schema_node.begin(); it != schema_node.end(); ++it)
			{
				// If the current element is a sequence and has size greater than
				// one then process the deeper level.
				if (it->IsSequence())
				{
					if (it->size() > 1)
					{
						if (!ProcessNode(*test_current, *it, log_output))
							return false;
					}
					else
					{
						if (!TestSequence(*it, *test_current, log_output))
							return false;
					}
				}
				else
				{
					if (!ProcessNode(*test_current, *it, log_output))
						return false;
				}
				test_current++;
			}
		}
	}
	else if (schema_node.IsScalar())
	{
		// Verify the value against the schema type.
		if (!VerifyType(schema_node.as<std::string>(), test_node.as<std::string>()))
		{
			AddLogItem(log_output, LogLevel::INFO,
				"YamlSV::ProcessNode: Test value %s is not compatible with type %s",
				test_node.as<std::string>().c_str(),
				schema_node.as<std::string>().c_str());
			return false;
		}
	}
	else
	{
		AddLogItem(log_output, LogLevel::INFO, 
			"YamlSV::ProcessNode: Schema node is not a map, sequence, or scalar");
		return false;
	}
	return true;
}

bool YamlSV::VerifyType(const std::string& str_type, const std::string& test_val)
{
	if (schema_string_type_set.count(str_type) == 0)
		return false;

	// No need to check string. Anything can be interpreted as a string.
	// Use switch for now in case we add more types later. This is currently
	// probably not as efficient as using if, else if, etc. If we add another
	// case or several then it will likely be more efficient than if, else if.
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
			bool_tolower_ = parse_text_.ToLower(test_val);
			if (!(bool_tolower_ == "true" || bool_tolower_ == "false"))
				return false;
		}
	}
	return true;
}

bool YamlSV::TestMapElement(YAML::const_iterator& schema_it, YAML::const_iterator& test_it,
	std::vector<LogItem>& log_output)
{
	// If the mapped type is a scalar, check the value against the schema.
	if (schema_it->second.IsScalar())
	{
		// Check if the string data type is valid.
		if (!CheckDataTypeString(schema_it->second.as<std::string>(), str_type_,
			is_opt_))
		{
			AddLogItem(log_output, LogLevel::INFO,
				"YamlSV::TestMapElement: Type \"%s\" invalid",
				schema_it->second.as<std::string>().c_str());
			return false;
		}

		if (!test_it->second.IsScalar())
		{
			// If the test value is optional, indicated by is_opt_ = true,
			// return true if the test value is null.
			if (is_opt_)
			{
				if (test_it->second.IsNull())
				{
					AddLogItem(log_output, LogLevel::DDEBUG,
						"YamlSV::TestMapElement: Test value for key \"%s\" is optional and not present",
						test_it->first.as<std::string>().c_str());
					return true;
				}
			}
			AddLogItem(log_output, LogLevel::INFO,
				"YamlSV::TestMapElement: Value for key \"%s\" is not a scalar as"
				" indicated by the type \"%s\"",
				test_it->first.as<std::string>().c_str(),
				schema_it->second.as<std::string>().c_str());
			return false;
		}

		AddLogItem(log_output, LogLevel::DDEBUG,
			"YamlSV::TestMapElement: Testing value \"%s\" for key \"%s\", type \"%s\"",
			test_it->second.as<std::string>().c_str(),
			test_it->first.as<std::string>().c_str(),
			schema_it->second.as<std::string>().c_str());


		if (!VerifyType(str_type_, test_it->second.as<std::string>()))
		{
			AddLogItem(log_output, LogLevel::INFO,
				"YamlSV::TestMapElement: Value for key \"%s\" does not match type \"%s\"",
				test_it->first.as<std::string>().c_str(), 
				str_type_.c_str());
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

bool YamlSV::TestSequence(const YAML::Node& schema_node, const YAML::Node& test_node,
	std::vector<LogItem>& log_output)
{
	// Ensure size of schema_node is 1.
	if (schema_node.size() != 1)
	{
		AddLogItem(log_output, LogLevel::INFO,
			"YamlSV::TestSequence: Schema node must have one element only (not %zu)",
			schema_node.size());
		return false;
	}

	// Error if the first (only) element in the schema is null.
	if (schema_node[0].IsNull())
	{
		AddLogItem(log_output, LogLevel::INFO,
			"YamlSV::TestSequence: Schema node element is null");
		return false;
	}

	// Before iterating over the test sequence, check if the string type
	// is valid.
	if (!CheckDataTypeString(schema_node[0].as<std::string>(), str_type_,
		is_opt_))
	{
		AddLogItem(log_output, LogLevel::INFO,
			"YamlSV::TestSequence: Type \"%s\" invalid", 
			schema_node[0].as<std::string>().c_str());
		return false;
	}

	// Can not evaluate if test node is a scalar.
	if (test_node.IsScalar())
	{
		AddLogItem(log_output, LogLevel::INFO,
			"YamlSV::TestSequence: Test node must be a sequence, not scalar with value \"%s\"",
			test_node.as<std::string>().c_str());
		return false;
	}

	// Size of the test sequence must be greater than zero.
	if (!(test_node.size() > 0))
	{
		// Unless it is an optional sequence.
		if (is_opt_)
		{
			AddLogItem(log_output, LogLevel::DDEBUG,
				"YamlSV::TestSequence: Test node is size zero and schema indicates OPT");
			return true;
		}
		AddLogItem(log_output, LogLevel::INFO,
			"YamlSV::TestSequence: Test node must have size greater than zero");
		return false;
	}

	AddLogItem(log_output, LogLevel::DDEBUG,
		"YamlSV::TestSequence: Test sequence of size %zu for type \"%s\"",
		test_node.size(), str_type_.c_str());
	
	// Iterate over the test_node sequence, checking the type of 
	// each element against the schema type.
	for (YAML::const_iterator it = test_node.begin(); it != test_node.end(); ++it)
	{
		if (!VerifyType(str_type_, it->as<std::string>()))
		{
			AddLogItem(log_output, LogLevel::INFO,
				"YamlSV::TestSequence: Value \"%s\" does not match type \"%s\"",
				it->as<std::string>().c_str(), str_type_.c_str());
			return false;
		}
	}
	return true;
}

bool YamlSV::CheckDataTypeString(const std::string& test_type, std::string& str_type,
	bool& is_opt)
{
	size_t modifier_pos = 0;
	if (schema_string_type_set.count(test_type) == 1)
	{
		str_type = test_type;
		is_opt = false;
		return true;
	}
	else if ((modifier_pos = test_type.find("OPT")) != std::string::npos)
	{
		if (modifier_pos == 0)
		{
			is_opt = true;

			// Remove the special chars.
			std::string schema_type = test_type.substr(3);
			if (schema_string_type_set.count(schema_type) == 1)
			{
				str_type = schema_type;
				return true;
			}
		}
	}
	
	str_type = "";
	is_opt = false;
	return false;
}