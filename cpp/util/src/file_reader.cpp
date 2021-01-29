#include "file_reader.h"

int FileReader::ReadFile(std::string file_name)
{
	// Connect to the file
	std::string line;
	std::ifstream in_file(file_name);

	// Check if file is valid
	if (!in_file.is_open())
		return 1;
	
	while (std::getline(in_file, line))
	{
		lines.push_back(line);
	}
	return 0;
}
