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
