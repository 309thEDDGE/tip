#ifndef FILE_READER_H
#define FILE_READER_H

#include <vector>
#include <string>
#include <fstream>

class FileReader
{

private:
	std::vector<std::string> lines;

public:
	FileReader() {};

	std::vector<std::string> GetLines() {return lines;};
	int ReadFile(std::string);
	
};

#endif