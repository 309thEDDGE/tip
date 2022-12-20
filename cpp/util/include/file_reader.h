#ifndef FILE_READER_H
#define FILE_READER_H

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "sysexits.h"

class FileReader
{
   private:
    std::vector<std::string> lines;

   public:
    FileReader() {}
    virtual ~FileReader() {}

    virtual std::vector<std::string> GetLines() { return lines; }
    virtual int ReadFile(std::string file_name);

    /*
	Get a string containing the entire document as if it 
	had not been read in line by line. Newline characters are
	added to the end of each line prior to be used in the 
	final document string.

	Return: String representation of the file read in ReadFile
	containing new line character at the end of each line.
	*/
    virtual std::string GetDocumentAsString();
};

#endif