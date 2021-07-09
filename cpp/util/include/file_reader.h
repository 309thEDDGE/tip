#ifndef FILE_READER_H
#define FILE_READER_H

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

class FileReader
{
   private:
    std::vector<std::string> lines;

   public:
    FileReader() {}

    std::vector<std::string> GetLines() { return lines; }
    int ReadFile(std::string);

    /*
	Get a string containing the entire document as if it 
	had not been read in line by line. Newline characters are
	added to the end of each line prior to be used in the 
	final document string.

	Return: String representation of the file read in ReadFile
	containing new line character at the end of each line.
	*/
    std::string GetDocumentAsString();
};

#endif