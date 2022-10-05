#ifndef FILE_READER_MOCK_H_
#define FILE_READER_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "file_reader.h"

class MockFileReader : public FileReader
{
   public:
    MockFileReader() : FileReader() {}
    MOCK_METHOD1(ReadFile, int(std::string file_name));
    MOCK_METHOD0(GetLines, std::vector<std::string>());
    MOCK_METHOD0(GetDocumentAsString, std::string());
};


#endif  // FILE_READER_MOCK_H_
