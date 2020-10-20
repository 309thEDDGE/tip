#include "dts1553.h"

bool DTS1553::IngestLines(const std::string& dts_path, const std::vector<std::string>& lines)
{
	// Check if yaml or text file.
	bool is_yaml = icd_data_.IsYamlFile(dts_path);

	// If yaml file, interpret lines as yaml and handle each component
	// with intended object. Otherwise pass all lines to ICDData.
	if (is_yaml)
	{
		printf("DTS1553::IngestLines(): Handling yaml file data\n");

		// Obtain each DTS1553 component as a yaml node.
		YAML::Node msg_defs;
		YAML::Node suppl_busmap;
		if (!ProcessLinesAsYaml(lines, msg_defs, suppl_busmap))
		{
			printf("DTS1553::IngestLines(): Process yaml lines failure!\n");
			return false;
		}

		if (!icd_data_.PrepareICDQuery(msg_defs))
		{
			printf("DTS1553::IngestLines(): PrepareICDQuery failure!\n");
			return false;
		}

		// If the supplemental bus map command words node has a size greater
		// than zero, fill the private member map.
		if (!FillSupplBusNameToMsgKeyMap(suppl_busmap, suppl_bus_name_to_message_key_map_))
		{
			printf("DTS1553::IngestLines(): Failed to generate bus name to message key map!\n");
			return false;
		}
	}
	else
	{
		printf("DTS1553::IngestLines(): Handling text/csv file data\n");
		if (!icd_data_.PrepareICDQuery(lines))
		{
			printf("DTS1553::IngestLines(): Failed to parse input lines!\n");
			return false;
		}
	}

	return true;
}

bool DTS1553::ProcessLinesAsYaml(const std::vector<std::string>& lines,
	YAML::Node& transl_msg_defs_node,
	YAML::Node& suppl_busmap_comm_words_node)
{
	// Bad if there are zero lines.
	if (lines.size() == 0)
	{
		printf("DTS1553::ProcessLinesAsYaml(): Input lines vector has size 0\n");
		return false;
	}

	// Concatenate all lines into a single string. It is requisite to append
	// to each line the newline character. Yaml loader must see new lines to
	// understand context.
	std::stringstream ss;
	std::for_each(lines.begin(), lines.end(),
		[&ss](const std::string& s) { ss << s; ss << "\n"; });
	std::string all_lines = ss.str();

	YAML::Node root_node = YAML::Load(all_lines.c_str());

	// Root node must have at least one entry.
	if (root_node.size() == 0)
	{
		printf("DTS1553::ProcessLinesAsYaml(): Root note has size 0\n");
		return false;
	}

	// Root node must be a map because all root-level items are maps.
	if (!root_node.IsMap())
	{
		printf("DTS1553::ProcessLinesAsYaml(): Root node is not a map\n");
		return false;
	}

	// The message definitions map MUST be present.
	std::string key_name = "";
	bool message_definitions_exist = false;
	for (YAML::const_iterator it = root_node.begin(); it != root_node.end(); ++it)
	{
		key_name = it->first.as<std::string>();
		if (yaml_key_to_component_map_.count(key_name) != 0)
		{
			switch (yaml_key_to_component_map_.at(key_name))
			{
			case DTS1553Component::TRANSL_MESSAGE_DEFS:
				message_definitions_exist = true;
				transl_msg_defs_node = it->second;
				break;
			case DTS1553Component::SUPPL_BUSMAP_COMM_WORDS:
				suppl_busmap_comm_words_node = it->second;
				break;
			}
		}
	}

	if (!message_definitions_exist)
	{
		printf("DTS1553::ProcessLinesAsYaml(): Message definitions node not present!\n");
		return false;
	}

	return true;
}

bool DTS1553::FillSupplBusNameToMsgKeyMap(const YAML::Node& suppl_busmap_comm_words_node,
	std::map<std::string, std::set<uint64_t>>& output_suppl_busname_to_msg_key_map)
{
	if (suppl_busmap_comm_words_node.size() == 0)
		return true;

	// Root node must be a map.
	if (!suppl_busmap_comm_words_node.IsMap())
	{
		printf("DTS1553::FillSupplBusNameToMsgKeyMap(): Root node is not a map\n");
		return false;
	}

	for (YAML::Node::const_iterator it = suppl_busmap_comm_words_node.begin();
		it != suppl_busmap_comm_words_node.end(); ++it)
	{
		// Fail if the value part of each mapping is not a sequence.
		if (!it->second.IsSequence())
		{
			printf("DTS1553::FillSupplBusNameToMsgKeyMap(): Value of mapping is not a sequence\n");
			return false;
		}
	}
	return true;
}