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

    MOCK_CONST_METHOD0(GetWorkerPathVec, 
        const std::vector<std::map<Ch10PacketType, ManagedPath>>&());

    MOCK_CONST_METHOD0(GetCh10PacketTypeEnabledMap, const std::map<Ch10PacketType, bool>&());

    MOCK_METHOD4(CreateOutputPaths, bool(const ManagedPath& ch10_input_path,
            const ManagedPath& output_dir, const std::map<Ch10PacketType, bool>& packet_enabled_map,
            uint16_t total_worker_count));
};


#endif  // PARSER_PATHS_MOCK_H_