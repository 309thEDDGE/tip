#ifndef PARSER_PATHS_MOCK_H_
#define PARSER_PATHS_MOCK_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "parser_paths.h"


class MockParserPaths: public ParserPaths
{
   public:
    MockParserPaths() : ParserPaths() {}
    
    MOCK_CONST_METHOD0(GetCh10PacketTypeOutputDirMap, 
        const std::map<Ch10PacketType, ManagedPath>&());

    MOCK_CONST_METHOD0(GetCh10Path, const ManagedPath&());
};


#endif  // PARSER_PATHS_MOCK_H_