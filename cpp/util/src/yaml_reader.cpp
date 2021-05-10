#include "yaml_reader.h"

bool YamlReader::LinkFile(std::string file_name)
{
	// Connect to the file
	std::ifstream in_file(file_name);

	// Check if file is valid
	if (!in_file.is_open())
	{
		printf("\nConfig file %s not found\n", file_name.c_str());
		return false;
	}

	in_file.close();

	// Set YAML node
	try
	{
		node = YAML::LoadFile(file_name);
	}
	catch (...)
	{
		printf("\nInvalid Config file %s\n", file_name.c_str());
		return false;
	}
	

	return true;	
}


bool YamlReader::IngestYamlAsString(const std::string& yaml_matter)
{
	if (yaml_matter.size() == 0)
	{
		printf("YamlReader::IngestYamlAsString: Empty string\n");
		return false;
	}

	// Set YAML node
	std::stringstream ss;
	ss.str(yaml_matter);

	try
	{
		node = YAML::Load(ss);
	}
	catch (YAML::Exception& e)
	{
		printf("YamlReader::IngestYamlAsString: YAML exception: %s\n", e.what());
		return false;
	}
	return true;
}