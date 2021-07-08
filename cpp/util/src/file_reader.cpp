#include "file_reader.h"

int FileReader::ReadFile(std::string file_name)
{
    // Clear previously ready read lines.
    lines.clear();

    // Connect to the file
    std::string line;
    std::ifstream in_file(file_name);

    // Check if file is valid
    if (!in_file.is_open())
        return 1;

    while (std::getline(in_file, line, '\n'))
    {
        lines.push_back(line);
    }

    // Close file
    in_file.close();

    return 0;
}

std::string FileReader::GetDocumentAsString()
{
    std::stringstream ss;
    std::for_each(lines.begin(), lines.end(),
                  [&ss](const std::string& s)
                  {
                      ss << s;
                      ss << "\n";
                  });
    std::string doc = ss.str();

    return doc;
}